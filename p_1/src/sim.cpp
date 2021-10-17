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

static inline double calc_norm(struct obj &o_i, struct obj &o_j)
{
	double d_x;
	double d_y;
	double d_z;

	d_x = o_i.x - o_j.x;
	d_y = o_i.y - o_j.y;
	d_z = o_i.z - o_j.z;

	return sqrt(d_x * d_x + d_y * d_y + d_z * d_z);
}

static inline void calc_fgv(struct obj &o_i, struct obj &o_j)
{
	/* fgv_no_recalc = G * mi * mj / denom */
	double denom;
	double fgv_no_recalc;

	double fx;
	double fy;
	double fz;

	denom = calc_norm(o_i, o_j);
	denom = denom * denom * denom;
	fgv_no_recalc = 6.674e-11 * o_i.m * o_j.m / denom;

	fx = fgv_no_recalc * (o_j.x - o_i.x);
	o_i.fx = o_i.fx + fx;
	o_j.fx = o_j.fx - fx;

	fy = fgv_no_recalc * (o_j.y - o_i.y);
	o_i.fy = o_i.fy + fy;
	o_j.fy = o_j.fy - fy;

	fz = fgv_no_recalc * (o_j.z - o_i.z);
	o_i.fz = o_i.fz + fz;
	o_j.fz = o_j.fz - fz;
}

static inline void calc_vel(struct obj &o, double time_step)
{
	double accel_no_recalc;
	accel_no_recalc = time_step/o.m;
	/* v = vi + a * time_step = vi + F/m * time_step */
	o.vx = o.vx + accel_no_recalc * o.fx;
	o.vy = o.vy + accel_no_recalc * o.fy;
	o.vz = o.vz + accel_no_recalc * o.fz;
}

static inline void calc_pos(struct obj &o, double size_enclosure, double time_step)
{
	/* p = pi + v * time_step */
	o.x = o.x + o.vx * time_step;
	if (o.x >= size_enclosure) {
		o.x = size_enclosure;
		o.vx = -o.vx;
	} else if (o.x <= 0) {
		o.x = 0;
		o.vx = -o.vx;
	}

	o.y = o.y + o.vy * time_step;
	if (o.y >= size_enclosure) {
		o.y = size_enclosure;
		o.vy = -o.vy;
	} else if (o.y <= 0) {
		o.y = 0;
		o.vy = -o.vy;
	}

	o.z = o.z + o.vz * time_step;
	if (o.z >= size_enclosure) {
		o.z = size_enclosure;
		o.vz = -o.vz;
	} else if (o.z <= 0) {
		o.z = 0;
		o.vz = -o.vz;
	}
}

/* efectos colaterales: cambia tamaño del vector o_list */
static void collision_check(std::vector<struct obj> &o_list)
{
	/* itera en reverso porque facilita el loop sobre un vector
	 * que cambia la cantidad de elementos.
	 */
	for (int i = o_list.size() - 1; i >= 0; --i)
		for (int j = i - 1; j >= 0; --j)
			if (calc_norm(o_list[i], o_list[j]) < 1.0) {
				/* primer argumento de merge_obj() es j
				 * porque por consigna el primer objeto en la
				 * lista se combina con el segundo. al ser
				 * un loop en reverso, el primer objeto es j
				 */
				merge_obj(o_list, j, i);
				/* objeto en i ya no existe */
				i--;
			}
}

void simulate(std::vector<struct obj> &o_list, unsigned int num_iterations,
		double size_enclosure, double time_step)
{
	/* chequeo pre-primera iteracion */
	collision_check(o_list);
	for (int k = 0; k < num_iterations; ++k) {
		/* itera en orden porque collision_check() itera
		 * en reverso. mejora un poco la localidad temporal
		 * porque los primeros elementos de c/iteracion han sido accedidos
		 * recientemente
		 */
		for (int i = 0; i < o_list.size(); ++i) {
			for (int j = i + 1; j < o_list.size(); ++j)
				calc_fgv(o_list[i], o_list[j]);

			/* necesita fuerza para calcular aceleracion */
			calc_vel(o_list[i], time_step);

			/* ya no se necesita fuerza, limpiar para prox iteracion */
			o_list[i].fx = 0;
			o_list[i].fy = 0;
			o_list[i].fz = 0;

			calc_pos(o_list[i], size_enclosure, time_step);
		}

		collision_check(o_list); /* chequeo de final de c/iteracion */
	}
}

int main(int argc, char *argv[])
{
	struct args arg_list = {
		.num_objects = -1,
		.num_iterations = -1,
		.random_seed = -1,
		.size_enclosure = -1,
		.time_step = -1,
	};
	int ret;
	int tmp_int;
	double tmp_double;
	/*
	 * TODO:
	 * - Escribir output formateado en caso de exito y en caso de error.
	 */
	if (unlikely(argc != 6)) {
		log_error("Wrong number of parameters", -1);
	}

	if (likely(is_int(argv[1]) && (tmp_int = atoi(argv[1])) > 0))
		arg_list.num_objects = tmp_int;
	else
		log_error("Error: invalid number of objects", -1);

	if (likely(is_int(argv[2]) && (tmp_int = atoi(argv[2])) > 0))
		arg_list.num_iterations = tmp_int;
	else
		log_error("Error: invalid number of iterations", -1);

	if (likely(is_int(argv[3]) && (tmp_int = atoi(argv[3])) > 0))
		arg_list.random_seed = tmp_int;
	else
		log_error("Error: invalid number for random seed", -1);

	if (likely(is_double(argv[4]) && (tmp_double = strtod(argv[4], NULL)) > 0))
		arg_list.size_enclosure = tmp_double;
	else
		log_error("Error: invalid box size", -1);

	if (likely(is_double(argv[5]) && (tmp_double = strtod(argv[5], NULL)) > 0))
		arg_list.time_step = tmp_double;
	else
		log_error("Error: invalid time step", -1);

	std::vector<struct obj> o_list(arg_list.num_objects);
	init_obj_list(o_list, arg_list.random_seed, arg_list.size_enclosure);

	if (write_config("init_config.txt", arg_list.size_enclosure, arg_list.time_step, o_list))
		log_error("Error while trying to write to init_config.txt\n", -3);

	simulate(o_list, arg_list.num_iterations, arg_list.size_enclosure, arg_list.time_step);

	if (write_config("final_config.txt", arg_list.size_enclosure, arg_list.time_step, o_list))
		log_error("Error while trying to write to final_config.txt\n", -3);

	return 0;
}
