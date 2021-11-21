#include <cmath>
#include <omp.h>

#include "util.hpp"
#include "obj.hpp"

#ifndef OMP_NUM_THREADS
#define OMP_NUM_THREADS 8
#endif

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
	/* tratar de conseguir un tamaño de bloque tal que no hayan superposiciones entre bloques
	 * de threads (false sharing)
	 */
	const size_t b = nxt_pow_2(o_soa.len/OMP_NUM_THREADS);

#pragma omp parallel
	{
	std::vector<double> fgv_no_recalc(b); /* mejor vector(heap) que VLA(stack)? */
	std::vector<double> norm(b); /* mejor vector(heap) que VLA(stack)? */

	#pragma omp for ordered schedule(auto)
	for (size_t jj = i + 1; jj < o_soa.len; jj += b) {

		for (size_t j = jj; j < std::min(jj+b, o_soa.len); ++j) {
			/* vectorizar cambia el resultado asi que dejar
			 * calc_norm() aqui para que std::sqrt() en calc_norm()
			 * evite que el loop sea vectorizado.
			 */
			norm[j-jj]= calc_norm(i, j, o_soa);
		}

		/* es lo unico que se puede vectorizar sin cambiar resultados.
		 * mide un leve speedup de todas maneras.
		 */
		#pragma omp simd
		for (size_t j = jj; j < std::min(jj+b, o_soa.len); ++j)
			fgv_no_recalc[j-jj] = 6.674e-11 * o_soa.m[i] * o_soa.m[j]
				/ (norm[j-jj] * norm[j-jj] * norm[j-jj]);

		#pragma omp ordered
		{
			double fx, fy, fz;
			fx = fy = fz = 0;
			for (size_t j = jj; j < std::min(jj+b, o_soa.len); ++j) {
				/* podria hacerse un array fx[b], fy[b], fz[b] y no dejar
				 * el calculo de los valores para esta parte, pero cambia resultado.
				 */
				fx = fgv_no_recalc[j-jj] * (o_soa.x[j] - o_soa.x[i]);
				fy = fgv_no_recalc[j-jj] * (o_soa.y[j] - o_soa.y[i]);
				fz = fgv_no_recalc[j-jj] * (o_soa.z[j] - o_soa.z[i]);

				o_soa.fx[j] = o_soa.fx[j] - fx;
				o_soa.fy[j] = o_soa.fy[j] - fy;
				o_soa.fz[j] = o_soa.fz[j] - fz;

				o_soa.fx[i] = o_soa.fx[i] + fx;
				o_soa.fy[i] = o_soa.fy[i] + fy;
				o_soa.fz[i] = o_soa.fz[i] + fz;
			}
		} /* fin #pragma omp ordered */
	}} /* fin #pragma omp for y #pragma omp parallel */
}

/* chequear inline. no es critico pero estaria cool */
static inline void calc_vel(size_t i, double time_step, struct soa &o_soa)
{
	double accel_no_recalc = time_step/o_soa.m[i];
	/* v = vi + a * time_step = vi + F/m * time_step */
	o_soa.vx[i] = o_soa.vx[i] + accel_no_recalc * o_soa.fx[i];
	o_soa.vy[i] = o_soa.vy[i] + accel_no_recalc * o_soa.fy[i];
	o_soa.vz[i] = o_soa.vz[i] + accel_no_recalc * o_soa.fz[i];
}

static inline void adjust_limits(size_t i, double size_enclosure,
		std::vector<double> &pos, std::vector<double> &vel)
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
}

static inline void mark(size_t idx, struct soa &o_soa)
{
	o_soa.m[idx] = 0;
}

void mark_collisions(struct soa &o_soa)
{
	/* paralelizable sin modificar orden de operaciones en merge_obj()?? */
	for (size_t i = 0ul; i < o_soa.len; ++i) {
		if (obj_marked(i, o_soa))
			continue;
		for (size_t j = i + 1ul; j < o_soa.len; ++j) {
			if (obj_marked(j, o_soa))
				continue;
			if (calc_norm(i, j, o_soa) < 1.0) {
				merge_obj(i, j, o_soa);
				mark(j, o_soa);
			}
		}
	}
}

void delete_marked(struct soa &o_soa)
{
	size_t last = 0ul;
	for (size_t i = 0ul; i < o_soa.len; ++i, ++last) {
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
	collision_check(o_soa);
	for (unsigned int k = 0; k < num_iterations; ++k) {
		for (size_t i = 0ul; i < o_soa.len; ++i) {
			calc_fgv(i, o_soa);
		}

/* leve speedup por paralelizacion de este loop */
#pragma omp parallel for schedule(guided)
		for (size_t i = 0ul; i < o_soa.len; ++i) {

			calc_vel(i, time_step, o_soa);

			/* ya no necesita fx, limpiar para prox iteracion */
			o_soa.fx[i] = 0;
			o_soa.fy[i] = 0;
			o_soa.fz[i] = 0;

			/* loop secuencial a partir de aca (cambio en posicion de un
			 * objeto puede afectar al resto
			 */
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
