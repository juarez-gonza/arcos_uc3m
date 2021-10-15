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

#include "obj.hpp"

#include <vector>

#include <iostream>

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

static void collision_check(std::vector<struct obj> &o_list)
{
	for (int i = 0; i < o_list.size(); ++i) {
		if (!obj_exists(o_list[i]))
			continue;
		for (int j = i + 1; j < o_list.size(); ++j) {
			if (!obj_exists(o_list[j]))
				continue;
			if (calc_norm(o_list[i], o_list[j]) < 1.0)
				merge_obj(o_list[i], o_list[j]);
		}
	}
}

void simulate(std::vector<struct obj> &o_list, unsigned int num_iterations,
		double size_enclosure, double time_step)
{

	collision_check(o_list); /* chequeo pre-primera iteracion */
	for (int k = 0; k < num_iterations; ++k) {
		for (int i = 0; i < o_list.size(); ++i) {
			if (!obj_exists(o_list[i]))
				continue;

			for (int j = i + 1; j < o_list.size(); ++j) {
				if (!obj_exists(o_list[j]))
					continue;
				calc_fgv(o_list[i], o_list[j]);
			}

			/* ya no se necesita fuerza, limpiar para prox iteracion */
			o_list[i].fx = 0;
			o_list[i].fy = 0;
			o_list[i].fz = 0;

			/* necesita fuerza para calcular aceleracion */
			calc_vel(o_list[i], time_step);
			calc_pos(o_list[i], size_enclosure, time_step);

		}
		collision_check(o_list); /* chequeo de final de c/iteracion */
	}
}

int main()
{
	unsigned int num_objects = 1000;
	unsigned int num_iterations = 50;
	unsigned int random_seed = 666;
	double size_enclosure = 1000000.0;
	double time_step = 0.1;

	/*
	 * TODO:
	 * - Escribir validacion de input
	 */
	std::vector<struct obj> o_list(num_objects);
	init_obj_list(o_list, random_seed, size_enclosure);
	/*
	 * TODO:
	 * - Escribir a init_fini.txt
	 */
	simulate(o_list, num_iterations, size_enclosure, time_step);

	/*
	 * TODO:
	 * - Escribir a final_conf.txt
	 */

	return 0;
}
