#include <iostream>

#include <stdlib.h>
#include <cstring>
#include <cmath>

#include <random>

#include <omp.h>

#define LINESIZE 64
#define ALIGNMENT 31
#define _align(_num) ((_num) + ALIGNMENT) & ~ALIGNMENT

struct soa {

	size_t len;
	double *m;
	double *x, *y, *z;
	double *fx, *fy, *fz;
	double *vx, *vy, *vz;

	soa(size_t ua_len, unsigned int random_seed, double upperbound)
	{

		std::mt19937_64 gen(random_seed);
		std::uniform_real_distribution<> uniform(0, upperbound);
		std::normal_distribution<> normal(1e+21, 1e+15);

		size_t size{_align(ua_len * sizeof(double))};

		len = ua_len;

		x = (double *)malloc(size);
		y = (double *)malloc(size);
		z = (double *)malloc(size);
		m = (double *)malloc(size);

		vx = (double *)malloc(size);
		memset(vx, 0, size);
		vy = (double *)malloc(size);
		memset(vy, 0, size);
		vz = (double *)malloc(size);
		memset(vy, 0, size);

		fx = (double *)malloc(size);
		memset(fx, 0, size);
		fy = (double *)malloc(size);
		memset(fy, 0, size);
		fz = (double *)malloc(size);
		memset(fy, 0, size);

		for (unsigned int i = 0; i < ua_len; ++i) {
			x[i] = uniform(gen);
			y[i] = uniform(gen);
			z[i] = uniform(gen);
			m[i] = normal(gen);
		}
	}

	~soa()
	{
		free(fz);
		free(fy);
		free(fx);
		free(vz);
		free(vy);
		free(vx);
		free(m);
		free(z);
		free(y);
		free(x);
	}
};

/*
 * chequear que de hecho el compilador haga el inline.
 * vectorizacion en calc_fgv() cuenta con esto
 */
static inline double calc_norm(size_t i, size_t j, struct soa &o_soa)
{
	double dx = o_soa.x[i] - o_soa.x[j];
	double dy = o_soa.y[i] - o_soa.y[j];
	double dz = o_soa.z[i] - o_soa.z[j];

	/*
	 * std::sqrt() no vectorizable a no ser que se use -fno-math-errno
	 * (ver -ffast-math y -Ofast). guarda: tratan FP como asociativo.
	 * si pc target no tiene simd necesarias, considerar dividir el loop aqui.
	 */
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

/* el compilador parece hacer inline de esto. no es necesario pero esta cool */
static void calc_fgv(size_t i, struct soa &o_soa)
{
	const size_t stride = 16ul; /* 16 tiene mejores resultados (por heuristica) */

	/* considerar sacar de este loop y colocar este loop header por encima
	 * del loop header del contador i (loop tiling). puede mejorar localidad
	 * y separacion de caché para prevenir false sharing */
	for (size_t jj = i + 1; jj < o_soa.len; jj += std::min(o_soa.len-jj, stride)) {

		/* asignar memoria dinamica en el stack: moralmente sucio */
		double fx[stride] __attribute__((aligned(64)));
		double fy[stride] __attribute__((aligned(64)));
		double fz[stride] __attribute__((aligned(64)));

		/* loop vectorizable */
		for (size_t j = jj; j < jj + std::min(o_soa.len-jj, stride); ++j) {
			double norm = calc_norm(i, j, o_soa);
			double denom = norm * norm * norm;
			double fgv_no_recalc = 6.674e-11 * o_soa.m[i] * o_soa.m[j] / denom;

			fx[j-jj] = fgv_no_recalc * (o_soa.x[j] - o_soa.x[i]);
			fy[j-jj] = fgv_no_recalc * (o_soa.y[j] - o_soa.y[i]);
			fz[j-jj] = fgv_no_recalc * (o_soa.z[j] - o_soa.z[i]);
			//printf("fgv_no_recalc: %f, fx[%d]: %f, fy[%d]: %f, fz[%d]: %f\n",
			//			fgv_no_recalc, j, fx[j], j, fy[j], j, fz[j]);
		}

		/* loop no vectorizable */
		for (size_t j = jj; j < jj + std::min(o_soa.len-jj, stride); ++j) {
			o_soa.fx[j] = o_soa.fx[j] - fx[j-jj];
			o_soa.fy[j] = o_soa.fy[j] - fy[j-jj];
			o_soa.fz[j] = o_soa.fz[j] - fz[j-jj];

			o_soa.fx[i] = o_soa.fx[i] + fx[j-jj];
			o_soa.fy[i] = o_soa.fy[i] + fy[j-jj];
			o_soa.fz[i] = o_soa.fz[i] + fz[j-jj];
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
	//printf("o_soa.x[%d]: %f\to_soa.y[%d]: %f\to_soa.z[%d]: %f\n",
	//		i, o_soa.x[i], i, o_soa.y[i], i, o_soa.z[i]);
}

int main()
{
	unsigned int num_objects = 2000;
	unsigned int num_iterations = 100;
	unsigned int random_seed = 666;
	double size_enclosure = 2000;
	double time_step = 0.1;

	struct soa o_soa{num_objects, random_seed, size_enclosure};

	double tic = omp_get_wtime();

	for (unsigned int k = 0; k < num_iterations; ++k) {
		/*
		 * considerar mine striping al loop header de i. probable mejora
		 * de paralelismo del algoritmo (quizas para prevenir false sharing).
		 * si loop header de jj se trae aqui, considerar loop collapse
		 * (y comparar performance jugando con el orden de loop headers ii y jj)
		 */
		for (size_t i = 0; i < o_soa.len; ++i) {

			calc_fgv(i, o_soa);
			calc_vel(i, time_step, o_soa);

			/* ya no necesita fx, limpiar para prox iteracion */
			o_soa.fx[i] = 0;
			o_soa.fy[i] = 0;
			o_soa.fz[i] = 0;

			calc_pos(i, time_step, size_enclosure, o_soa);

		}
	}

	double toc = omp_get_wtime();
	printf("tiempo de ejecucion: %f\n", toc-tic);
}
