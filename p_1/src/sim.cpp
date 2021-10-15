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

#include "obj.h"

#include <vector>

#include <iostream>

inline double calc_norm(struct obj &o_i, struct obj &o_j)
{
	double d_x;
	double d_y;
	double d_z;

	d_x = o_i.x - o_j.x;
	d_y = o_i.y - o_j.y;
	d_z = o_i.z - o_j.z;

	return sqrt(d_x * d_x + d_y * d_y + d_z * d_z);
}

inline void calc_fgv(struct obj &o_i, struct obj &o_j)
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

	fx = fgv_no_recalc * (o_i.x - o_j.x);
	o_i.fx += fx;
	o_j.fx -= fx;

	fy = fgv_no_recalc * (o_i.y - o_j.y);
	o_i.fy += fy;
	o_j.fy -= fy;

	fz = fgv_no_recalc * (o_i.z - o_j.z);
	o_i.fz += fz;
	o_j.fz -= fz;
}

inline void calc_vel(struct obj &o, double time_step)
{
	double accel_no_recalc;

	accel_no_recalc = time_step/o.m;
	/* v = vi + a * time_step = vi + F/m * time_step */
	o.x += accel_no_recalc * o.fx;
	o.y += accel_no_recalc * o.fy;
	o.z += accel_no_recalc * o.fz;
}

void calc_pos(struct obj &o, double size_enclosure, double time_step)
{
	/* p = pi + v * time_step */
	o.x += o.vx * time_step;
	if (o.x >= size_enclosure)
		o.x = size_enclosure;
	if (o.x <= 0)
		o.x = 0;

	o.y += o.vy * time_step;
	if (o.y >= size_enclosure)
		o.y = size_enclosure;
	if (o.y <= 0)
		o.y = 0;

	o.z += o.vz * time_step;
	if (o.z >= size_enclosure)
		o.z = size_enclosure;
	if (o.z <= 0)
		o.z = 0;
}

void collision_check(std::vector<struct obj> &o_list)
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
	/*
	 * TODO:
	 * - Implement num_iterations
	 */

	collision_check(o_list); /* chequeo pre-primera iteracion */
	for (int k = 0; k < num_iterations; ++k) {
		for (int i = 0; i < o_list.size(); ++i) {
			if (!obj_exists(o_list[i]))
				continue;
			/*
			std::cout << "=======================\n"
			<< "pasando " << o_list[i] << "\n";
			*/
			for (int j = i + 1; j < o_list.size(); ++j) {
				if (!obj_exists(o_list[j]))
					continue;
				/*
				std::cout << "\tcon " << o_list[j] << "\n";
				*/
				calc_fgv(o_list[i], o_list[j]);
			}

			calc_vel(o_list[i], time_step);
			calc_pos(o_list[i], size_enclosure, time_step);

			/*
			std::cout << "o:\t" << o_list[i] << "\n\tpos_x: " << o_i.x
			<< "\n\texists?: " << o_list[i].exists
			<< "\n\tpos_y: " << o_i.y
			<< "\n\tpos_z: " << o_i.z
			<< "\n\tm: " << o_i.m
			<< "\n\tvel_x: " << o_list[i].vel.x
			<< "\n\tvel_y: " << o_list[i].vel.y
			<< "\n\tvel_z: " << o_list[i].vel.z
			<< "\n\tfgv_x: " << o_i.fx
			<< "\n\tfgv_y: " << o_i.fy
			<< "\n\tfgv_z: " << o_i.fz << "\n";
			*/
		}
		collision_check(o_list); /* chequeo de final de c/iteracion */
	}
}

int main()
{
	unsigned int num_objects = 1000;
	unsigned int num_iterations = 20;
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
error:
	return 1;
}
