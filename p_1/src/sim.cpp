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

#include <iostream>

inline void calc_fgv(struct obj *o_i, struct obj *o_j)
{
	/* 1/(||pj - pi||^3) == 1/(sqrt(pj-pi)^3)
	 * == 1/((pj-pi)^(3/2)) == (pj-pi)**(-3/2)
	 */
	double denom;
	/* fgv_no_recalc = G * mi * mj * denom */
	double fgv_no_recalc;
	double fx;
	double fy;
	double fz;

	denom = pow(sqrt(pow(o_i->pos->x - o_j->pos->x, 2)
		+ pow(o_i->pos->y - o_j->pos->y, 2)
		+ pow(o_i->pos->z - o_j->pos->z, 2)), 3);
	fgv_no_recalc = 6.674e-11 * o_i->m * o_j->m / denom;

	fx = fgv_no_recalc * (o_i->pos->x - o_j->pos->x);
	o_i->fgv->x += fx;
	o_j->fgv->x -= fx;

	fy = fgv_no_recalc * (o_i->pos->y - o_j->pos->y);
	o_i->fgv->y += fy;
	o_j->fgv->y -= fy;

	fz = fgv_no_recalc * (o_i->pos->z - o_j->pos->z);
	o_i->fgv->z += fz;
	o_j->fgv->z -= fz;
}

inline void calc_vel(struct obj *o, double time_step)
{
	double accel_no_recalc;

	accel_no_recalc = time_step/o->m;
	/* v = vi + a * time_step = vi + F/m * time_step */
	o->vel->x += accel_no_recalc * o->fgv->x;
	o->vel->y += accel_no_recalc * o->fgv->y;
	o->vel->z += accel_no_recalc * o->fgv->z;
}

inline void calc_pos(struct obj *o, double size_enclosure, double time_step)
{
	/* p = pi + v * time_step */
	o->pos->x += o->vel->x * time_step;
	if (o->pos->x >= size_enclosure)
		o->pos->x = size_enclosure;
	if (o->pos->x <= 0)
		o->pos->x = 0;

	o->pos->y += o->vel->y * time_step;
	if (o->pos->y >= size_enclosure)
		o->pos->y = size_enclosure;
	if (o->pos->y <= 0)
		o->pos->y = 0;

	o->pos->z += o->vel->z * time_step;
	if (o->pos->z >= size_enclosure)
		o->pos->z = size_enclosure;
	if (o->pos->z <= 0)
		o->pos->z = 0;
}

void simulate(struct obj_list *o_listp, unsigned int num_iterations,
		double size_enclosure, double time_step)
{
	struct obj *o_i;
	struct obj *o_j;
	struct obj *last_addr;
	/*
	 * TODO:
	 * - Implement collisions
	 * - Implement num_iterations
	 */

	last_addr = o_listp->list + o_listp->size;
	for (o_i = o_listp->list; o_i != last_addr; ++o_i) {
		/*
		std::cout << "=======================\n"
		<< "pasando " << o_i << "\n";
		*/
		for (o_j = o_i + 1; o_j != last_addr; ++o_j) {
			/*
			std::cout << "\tcon " << o_j << "\n";
			*/
			calc_fgv(o_i, o_j);
		}

		calc_vel(o_i, time_step);
		calc_pos(o_i, size_enclosure, time_step);
		/*
		std::cout << "o:\t" << o_i << "\n\tpos_x: " << o_i->pos->x
		<< "\n\tpos_y: " << o_i->pos->y
		<< "\n\tpos_z: " << o_i->pos->z
		<< "\n\tm: " << o_i->m
		<< "\n\tvel_x: " << o_i->vel->x
		<< "\n\tvel_y: " << o_i->vel->y
		<< "\n\tvel_z: " << o_i->vel->z
		<< "\n\tfgv_x: " << o_i->fgv->x
		<< "\n\tfgv_y: " << o_i->fgv->y
		<< "\n\tfgv_z: " << o_i->fgv->z << "\n";
		*/
	}
}

int main()
{
	unsigned int num_objects = 2;
	unsigned int num_iterations = 1;
	unsigned int random_seed = 666;
	double size_enclosure = 20.0;
	double time_step = 1;

	/*
	 * TODO:
	 * - Write input validation
	 */

	struct obj_list o_list;
	if (init_obj_list(&o_list, num_objects,
			random_seed, size_enclosure))
		goto error;
	/*
	 * TODO:
	 * - Write init_conf.txt file
	 */
	simulate(&o_list, num_iterations, size_enclosure, time_step);

	destroy_obj_list(&o_list);
	return 0;
error:
	return 1;
}
