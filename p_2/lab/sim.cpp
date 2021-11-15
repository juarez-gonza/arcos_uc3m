#include <cmath>

#include <omp.h>

#include "util.hpp"
#include "obj.hpp"

/* chequear que de hecho el compilador haga inline de calc_norm().
 * vectorizacion en calc_fgv() cuenta con inline.
 */
static inline double calc_norm(size_t i, size_t j, struct soa &o_soa)
{
	double dx = o_soa.x[i] - o_soa.x[j];
	double dy = o_soa.y[i] - o_soa.y[j];
	double dz = o_soa.z[i] - o_soa.z[j];

	/* std::sqrt() parece ser no vectorizable a no ser que se use -fno-math-errno
	 * (ver -ffast-math y -Ofast). guarda: tratan FP como asociativo y aproximan por
	 * metodos numericos.
	 */
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

/* el compilador parece hacer inline de esto. no es necesario pero esta cool */
static void calc_fgv(size_t i, struct soa &o_soa)
{
	const size_t b = 2048ul; /* 2048 tiene mejores resultados (por heuristica) */

	for (size_t jj = i + 1; jj < o_soa.len; jj += b) {
		/* asignar memoria dinamica en el stack: moralmente sucio */
		double norm[b] __attribute__((aligned(64)));
		double fgv_no_recalc[b] __attribute__((aligned(64)));

		for (size_t j = jj; j < std::min(jj+b, o_soa.len); ++j) {
			norm[j-jj] = calc_norm(i, j, o_soa);
			fgv_no_recalc[j-jj] = 6.674e-11 * o_soa.m[i] * o_soa.m[j]
				/ (norm[j-jj] * norm[j-jj] * norm[j-jj]);

			o_soa.fx[j] = o_soa.fx[j] - fgv_no_recalc[j-jj] * (o_soa.x[j] - o_soa.x[i]);
			o_soa.fy[j] = o_soa.fy[j] - fgv_no_recalc[j-jj] * (o_soa.y[j] - o_soa.y[i]);
			o_soa.fz[j] = o_soa.fz[j] - fgv_no_recalc[j-jj] * (o_soa.z[j] - o_soa.z[i]);

			o_soa.fx[i] = o_soa.fx[i] + fgv_no_recalc[j-jj] * (o_soa.x[j] - o_soa.x[i]);
			o_soa.fy[i] = o_soa.fy[i] + fgv_no_recalc[j-jj] * (o_soa.y[j] - o_soa.y[i]);
			o_soa.fz[i] = o_soa.fz[i] + fgv_no_recalc[j-jj] * (o_soa.z[j] - o_soa.z[i]);
		}
	}
	//printf("o_soa.fx[%d]: %f\to_soa.fy[%d]: %f\to_soa.fz[%d]: %f\n",
	//		i, o_soa.fx[i], i, o_soa.fy[i], i, o_soa.fz[i]);
}

/* chequear inline. no es critico pero estaria cool */
static inline void calc_vel(size_t i, double time_step, struct soa &o_soa)
{
	double accel_no_recalc = time_step/o_soa.m[i];
	/* v = vi + a * time_step = vi + F/m * time_step */
	o_soa.vx[i] = o_soa.vx[i] + accel_no_recalc * o_soa.fx[i];
	o_soa.vy[i] = o_soa.vy[i] + accel_no_recalc * o_soa.fy[i];
	o_soa.vz[i] = o_soa.vz[i] + accel_no_recalc * o_soa.fz[i];
	//printf("o_soa.vx[%d]: %f\to_soa.vy[%d]: %f\to_soa.vz[%d]: %f\n",
	//		i, o_soa.vx[i], i, o_soa.vy[i], i, o_soa.vz[i]);
}

static inline void adjust_limits(size_t i, double size_enclosure,
		double pos[], double vel[])
{
	if (pos[i] >= size_enclosure) {
		pos[i] = size_enclosure;
		vel[i] = -vel[i];
	} else if (pos[i] <= 0) {
		pos[i] = 0;
		vel[i] = -vel[i];
	}
}

/* el compilador parece hacer inline de esto. no es necesario pero esta cool */
static void calc_pos(size_t i, double time_step, double size_enclosure, struct soa &o_soa)
{
	/* p = pi + v * time_step */
	o_soa.x[i] = o_soa.x[i] + o_soa.vx[i] * time_step;
	adjust_limits(i, size_enclosure, o_soa.x, o_soa.vx);

	o_soa.y[i] = o_soa.y[i] + o_soa.vy[i] * time_step;
	adjust_limits(i, size_enclosure, o_soa.y, o_soa.vy);

	o_soa.z[i] = o_soa.z[i] + o_soa.vz[i] * time_step;
	adjust_limits(i, size_enclosure, o_soa.z, o_soa.vz);
	//printf("o_soa.x[]: %f\to_soa.y[]: %f\to_soa.z[]: %f\n",
	//		o_soa.x[i], o_soa.y[i], o_soa.z[i]);
}

static inline void mark(size_t idx, struct soa &o_soa)
{
	o_soa.m[idx] = 0;
}

void mark_collisions(struct soa &o_soa)
{
	for (size_t i = 0; i < o_soa.len; ++i) {
		if (obj_marked(i, o_soa))
			continue;
		for (size_t j = i + 1; j < o_soa.len; ++j) {
			if (obj_marked(j, o_soa))
				continue;
			if (calc_norm(i, j, o_soa) < 1.0) {
				merge_obj(last, i, o_soa);
				mark(j, o_soa);
			}
		}
	}
}

void delete_marked(struct soa &o_soa)
{
	size_t last = 0;
	for (size_t i = 0; i < o_soa.len; ++i, ++last) {
		while (obj_marked(i, o_soa) && i < o_soa.len) {
			i++;
		}
		if (i >= o_soa.len)
			break;
		obj_copy_into(last, i, o_soa);
	}
	o_soa.len = last;
}

static void inline collision_check(struct soa &o_soa)
{
	mark_collisions(o_soa);
	delete_marked(o_soa);
}

void simulate(struct soa &o_soa, unsigned int num_iterations,
		double size_enclosure, double time_step)
{
	const size_t b = 2048ul; /* 16 tiene mejores resultados (por heuristica) */

	collision_check(o_soa);
	for (unsigned int k = 0; k < num_iterations; ++k) {
//#pragma omp parallel for schedule(auto)
		for (size_t ii = 0; ii < o_soa.len; ii+=b) {
		for (size_t i = ii; i < std::min(ii+b, o_soa.len); ++i) {
		//for (size_t i = 0ul; i < o_soa.len; ++i) {
			calc_fgv(i, o_soa);
		}} /* fin ii parallel for */

		for (size_t i = 0ul; i < o_soa.len; ++i) {
			calc_vel(i, time_step, o_soa);

			/* ya no necesita fx, limpiar para prox iteracion */
			o_soa.fx[i] = 0;
			o_soa.fy[i] = 0;
			o_soa.fz[i] = 0;

			calc_pos(i, time_step, size_enclosure, o_soa);
		}

		collision_check(o_soa);
	 }
}

int main(int argc, char *argv[])
{
	struct args arg_list{};

	parse_args(arg_list, argc, argv);

	struct soa o_soa(arg_list.num_objects,
		arg_list.random_seed, arg_list.size_enclosure);

	if (write_config("init_config.txt", arg_list.size_enclosure, arg_list.time_step, o_soa))
		log_n_exit("Error while trying to write to init_config.txt\n", 1);


	double tic = omp_get_wtime();

	simulate(o_soa, arg_list.num_iterations,
		arg_list.size_enclosure, arg_list.time_step);

	double toc = omp_get_wtime();

	printf("tiempo de ejecucion: %f\n", toc-tic);

	if (write_config("final_config.txt", arg_list.size_enclosure, arg_list.time_step, o_soa))
		log_n_exit("Error while trying to write to final_config.txt\n", 1);
}
