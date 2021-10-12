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

inline double calc_norm(struct obj *o_ip, struct obj *o_jp)
{
	return sqrt(pow(o_ip->pos.x - o_jp->pos.x, 2)
		+ pow(o_ip->pos.y - o_jp->pos.y, 2)
		+ pow(o_ip->pos.z - o_jp->pos.z, 2));
}

inline void calc_fgv(struct obj *o_ip, struct obj *o_jp)
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

	denom = pow(calc_norm(o_ip, o_jp), 3);
	fgv_no_recalc = 6.674e-11 * o_ip->m * o_jp->m / denom;

	fx = fgv_no_recalc * (o_ip->pos.x - o_jp->pos.x);
	o_ip->fgv.x += fx;
	o_jp->fgv.x -= fx;

	fy = fgv_no_recalc * (o_ip->pos.y - o_jp->pos.y);
	o_ip->fgv.y += fy;
	o_jp->fgv.y -= fy;

	fz = fgv_no_recalc * (o_ip->pos.z - o_jp->pos.z);
	o_ip->fgv.z += fz;
	o_jp->fgv.z -= fz;
}

inline void calc_vel(struct obj *op, double time_step)
{
	double accel_no_recalc;

	accel_no_recalc = time_step/op->m;
	/* v = vi + a * time_step = vi + F/m * time_step */
	op->vel.x += accel_no_recalc * op->fgv.x;
	op->vel.y += accel_no_recalc * op->fgv.y;
	op->vel.z += accel_no_recalc * op->fgv.z;
}

void calc_pos(struct obj *op, double size_enclosure, double time_step)
{
	/* p = pi + v * time_step */
	op->pos.x += op->vel.x * time_step;
	if (op->pos.x >= size_enclosure)
		op->pos.x = size_enclosure;
	if (op->pos.x <= 0)
		op->pos.x = 0;

	op->pos.y += op->vel.y * time_step;
	if (op->pos.y >= size_enclosure)
		op->pos.y = size_enclosure;
	if (op->pos.y <= 0)
		op->pos.y = 0;

	op->pos.z += op->vel.z * time_step;
	if (op->pos.z >= size_enclosure)
		op->pos.z = size_enclosure;
	if (op->pos.z <= 0)
		op->pos.z = 0;
}

void collision_check(struct obj_list *o_listp, struct obj *last_addr)
{
	struct obj *o_jp;
	struct obj *o_ip;

	last_addr = o_listp->list + o_listp->size;
	for (o_ip = o_listp->list; o_ip != last_addr; ++o_ip) {
		for (o_jp = o_ip + 1; o_jp != last_addr; ++o_jp) {
			if (!obj_exists(o_jp))
				continue;
			if (calc_norm(o_jp, o_ip) < 1.0)
				merge_obj(o_ip, o_jp);
		}
	}
}

void simulate(struct obj_list *o_listp, unsigned int num_iterations,
		double size_enclosure, double time_step)
{
	struct obj *o_ip;
	struct obj *o_jp;
	struct obj *last_addr;
	/*
	 * TODO:
	 * - Implement num_iterations
	 */
	last_addr = o_listp->list + o_listp->size;

	collision_check(o_listp, last_addr);
	for (o_ip = o_listp->list; o_ip != last_addr; ++o_ip) {
		/*
		std::cout << "=======================\n"
		<< "pasando " << o_ip << "\n";
		*/
		for (o_jp = o_ip + 1; o_jp != last_addr; ++o_jp) {
			if (obj_exists(o_jp) == 0)
				continue;
			/*
			std::cout << "\tcon " << o_jp << "\n";
			*/
			calc_fgv(o_ip, o_jp);
		}

		calc_vel(o_ip, time_step);
		calc_pos(o_ip, size_enclosure, time_step);
		/*
		std::cout << "o:\t" << o_ip << "\n\tpos_x: " << o_ip->pos.x
		<< "\n\tpos_y: " << o_ip->pos.y
		<< "\n\tpos_z: " << o_ip->pos.z
		<< "\n\tm: " << o_ip->m
		<< "\n\tvel_x: " << o_ip->vel.x
		<< "\n\tvel_y: " << o_ip->vel.y
		<< "\n\tvel_z: " << o_ip->vel.z
		<< "\n\tfgv_x: " << o_ip->fgv.x
		<< "\n\tfgv_y: " << o_ip->fgv.y
		<< "\n\tfgv_z: " << o_ip->fgv.z << "\n";
		*/
	}
	collision_check(o_listp, last_addr);
}

int main()
{
	unsigned int num_objects = 2;
	unsigned int num_iterations = 1;
	unsigned int random_seed = 666;
	double size_enclosure = 0.1;
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
