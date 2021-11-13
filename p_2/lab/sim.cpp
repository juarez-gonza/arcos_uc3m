#include <iostream>

#include <stdlib.h>
#include <cstring>
#include <cmath>
#include <limits>
#include <random>

#include <omp.h>

#define SIMD_ALIGNMENT 64 /* sirve para SSE, AVX, y AVX512 */
#define ALIGNMENT SIMD_ALIGNMENT
#define _align(_num) ((_num) + (ALIGNMENT-1)) & ~(ALIGNMENT-1)

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
		if (NULL == x)
			goto fail_x;
		y = (double *)malloc(size);
		if (NULL == y)
			goto fail_y;
		z = (double *)malloc(size);
		if (NULL == z)
			goto fail_z;
		m = (double *)malloc(size);
		if (NULL == m)
			goto fail_m;

		vx = (double *)malloc(size);
		if (NULL == vx)
			goto fail_vx;
		memset(vx, 0, size);
		vy = (double *)malloc(size);
		if (NULL == vy)
			goto fail_vy;
		memset(vy, 0, size);
		vz = (double *)malloc(size);
		if (NULL == vz)
			goto fail_vz;
		memset(vy, 0, size);

		fx = (double *)malloc(size);
		if (NULL == fx)
			goto fail_fx;
		memset(fx, 0, size);
		fy = (double *)malloc(size);
		if (NULL == fy)
			goto fail_fy;
		memset(fy, 0, size);
		fz = (double *)malloc(size);
		if (NULL == fz)
			goto fail_fz;
		memset(fy, 0, size);

		for (unsigned int i = 0; i < ua_len; ++i) {
			x[i] = uniform(gen);
			y[i] = uniform(gen);
			z[i] = uniform(gen);
			m[i] = normal(gen);
		}

		return;
fail_fz:
		free(fy);
fail_fy:
		free(fx);
fail_fx:
		free(vz);
fail_vz:
		free(vy);
fail_vy:
		free(vx);
fail_vx:
		free(m);
fail_m:
		free(z);
fail_z:
		free(y);
fail_y:
		free(x);
fail_x:
		throw std::bad_alloc();
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

/* chequear que de hecho el compilador haga inline de calc_norm().
 * vectorizacion en calc_fgv() cuenta con inline.
 */
#pragma omp declare simd
static inline double calc_norm(size_t i, size_t j, struct soa &o_soa)
{
	double dx = o_soa.x[i] - o_soa.x[j];
	double dy = o_soa.y[i] - o_soa.y[j];
	double dz = o_soa.z[i] - o_soa.z[j];

	/* std::sqrt() parece ser no vectorizable a no ser que se use -fno-math-errno
	 * (ver -ffast-math y -Ofast). guarda: tratan FP como asociativo y aproximar.
	 * si pc target no tiene simd necesarias, considerar dividir el loop aqui.
	 */
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

/* el compilador parece hacer inline de esto. no es necesario pero esta cool */
static void calc_fgv(struct soa &o_soa)
{
	const size_t b = 2048ul; /* 2048 parece tener mejores resultados en paralelismo (heuristica) */
	const size_t N = o_soa.len - 1ul;

/* paralelizar ii: fuerza resultante del conjunto de objetos es calculada "en simultaneo".
 * paralelizar jj: sumatoria de la fuerza resultante en un objeto es calculada "en simultaneo".
 */
#pragma omp parallel for schedule(auto)
	for (size_t ii = 0ul; ii <= N; ii += b) {
#pragma omp parallel for schedule(auto)
	for (size_t jj = ii+1ul; jj <= N; jj += b) {

		/* asignar memoria dinamica en el stack: moralmente sucio */
		double fx[b] __attribute__((aligned(ALIGNMENT)));
		double fy[b] __attribute__((aligned(ALIGNMENT)));
		double fz[b] __attribute__((aligned(ALIGNMENT)));
		double norm[b] __attribute__((aligned(ALIGNMENT)));
		double fgv_no_recalc[b] __attribute__((aligned(ALIGNMENT)));

		for (size_t i = ii; i <= std::min(ii+b-1ul, N); ++i) {
			/* separacion de loop para vectorizacion:
			 * Dependencia de flujo (RAW) entre asignar a f*[j-jj] y usarlo
			 * para la suma a las fuerzas de o_soa.f*[i] y o_soa.f*[j].
			 * Parece ser la causa por la que el compilador no vectoriza el loop en ese caso.
			 * Al dividir el calculo en 2 loops, el compilador puede vectorizar sin problemas.
			 *
			 * loop vectorizable:
			 * RAW en fgv_no_recalc[j-jj] y RAW en norm[j-jj] parece no impedir vectorizacion.
			 */
			#pragma omp simd
			for (size_t j = std::max(jj, i+1ul); j <= std::min(jj+b-1ul, N); ++j) {
				norm[j-jj] = calc_norm(i, j, o_soa);
				fgv_no_recalc[j-jj] = (6.674e-11 * o_soa.m[i] * o_soa.m[j])
					/ (norm[j-jj] * norm[j-jj] * norm[j-jj]);

				fx[j-jj] = fgv_no_recalc[j-jj] * (o_soa.x[j] - o_soa.x[i]);
				fy[j-jj] = fgv_no_recalc[j-jj] * (o_soa.y[j] - o_soa.y[i]);
				fz[j-jj] = fgv_no_recalc[j-jj] * (o_soa.z[j] - o_soa.z[i]);
			}

			/* loop vectorizable:
			 * Dependencia de output (WAW) en variables fxi, fyi, fzi,
			 * el compilador parece poder vectorizar el loop de todas maneras.
			 * No vectoriza cuando se reemplazan las variables f* por o_soa.f*[i].
			 *
			 * condiciones de carrera:
			 * Marcar el loop entero como zona critica va mucho mejor que adquirir lock
			 * en cada iteracion (loop no es muuuuy grande -> maximo de b iteraciones).
			 */
			double fxi, fyi, fzi;
			fxi = fyi = fzi = 0.0;
			#pragma omp critical
			{
			#pragma omp simd
			for (size_t j = std::max(jj, i+1ul); j <= std::min(jj+b-1ul, N); j++) {
				o_soa.fx[j] = o_soa.fx[j] - fx[j-jj];
				o_soa.fy[j] = o_soa.fy[j] - fy[j-jj];
				o_soa.fz[j] = o_soa.fz[j] - fz[j-jj];

				fxi = fxi + fx[j-jj];
				fyi = fyi + fy[j-jj];
				fzi = fzi + fz[j-jj];
			}
			o_soa.fx[i] += fxi;
			o_soa.fy[i] += fyi;
			o_soa.fz[i] += fzi;
			}
		}
	}} /* fin omp parallel ii y jj */
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

/* chequear inline. no es critico pero estaria cool */
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

/* chequear inline */
static inline void merge_obj(size_t i, size_t j, struct soa &o_soa)
{
	o_soa.m[i] += o_soa.m[j];
	o_soa.vx[i] += o_soa.vx[j];
	o_soa.vy[i] += o_soa.vy[j];
	o_soa.vz[i] += o_soa.vz[j];
}

/* el compilador parece hacer inline de esto. no es necesario pero esta cool */
static void obj_copy_into(size_t into_idx, size_t from_idx, struct soa &o_soa)
{
	o_soa.m[into_idx] = o_soa.m[from_idx];
	o_soa.x[into_idx] = o_soa.x[from_idx];
	o_soa.y[into_idx] = o_soa.y[from_idx];
	o_soa.z[into_idx] = o_soa.z[from_idx];

	o_soa.fx[into_idx] = o_soa.fx[from_idx];
	o_soa.fy[into_idx] = o_soa.fy[from_idx];
	o_soa.fz[into_idx] = o_soa.fz[from_idx];

	o_soa.vx[into_idx] = o_soa.vx[from_idx];
	o_soa.vy[into_idx] = o_soa.vy[from_idx];
	o_soa.vz[into_idx] = o_soa.vz[from_idx];
}

double double_max = std::numeric_limits<double>::max();
/* chequear inline */
static inline void mark_atomic(size_t idx, struct soa &o_soa)
{
#pragma omp atomic
	o_soa.m[idx] = o_soa.m[idx]-double_max;
}

/* chequear inline */
static inline bool obj_marked(size_t idx, struct soa &o_soa)
{
	return o_soa.m[idx] <= 0;
}

void mark_collisions(struct soa &o_soa)
{
	const size_t b = 128ul; /* 128 tiene mejores resultados (heuristica) */
	const size_t N = o_soa.len - 1ul;

	for (size_t ii = 0; ii <= N; ii += b) {
#pragma omp parallel for schedule(auto)
	for (size_t jj = ii+1; jj <= N; jj += b) {
		for (size_t i = ii; i <= std::min(ii+b-1ul, N); i++) {
			if (obj_marked(i, o_soa))
				continue;
			for (size_t j = std::max(jj, i+1ul); j <= std::min(jj+b-1ul, N); j++) {
				if (obj_marked(j, o_soa))
					continue;
				if (calc_norm(i, j, o_soa) < 1.0) {
					merge_obj(i, j, o_soa);
					mark_atomic(j, o_soa);
				}
			}
		}
	}} /* fin loop ii y parallel jj */
}

void delete_marked(struct soa &o_soa)
{
	size_t last = 0ul;
	for (size_t i = 0ul; i < o_soa.len; ++i, ++last) {
		while (obj_marked(i, o_soa) && i < o_soa.len)
			i++;
		if (i >= o_soa.len)
			break;
		obj_copy_into(last, i, o_soa);
	}
	o_soa.len = last;
}

static void collision_check(struct soa &o_soa)
{
	mark_collisions(o_soa);
	delete_marked(o_soa);
}

int main()
{
	unsigned int num_objects = 10000;
	unsigned int num_iterations = 250;
	unsigned int random_seed = 666;
	double size_enclosure = 2000;
	double time_step = 0.1;

	struct soa o_soa{num_objects, random_seed, size_enclosure};

	double tic = omp_get_wtime();

	collision_check(o_soa);
	for (unsigned int k = 0; k < num_iterations; ++k) {

		calc_fgv(o_soa);

		/* loop secuencial. paralelizar puede causar diferencias en posiciones */
		for (size_t i = 0ul; i < o_soa.len; ++i) {
			calc_vel(i, time_step, o_soa);

			/* ya no necesita f*[i], limpiar para prox iteracion */
			o_soa.fx[i] = 0ul;
			o_soa.fy[i] = 0ul;
			o_soa.fz[i] = 0ul;

			calc_pos(i, time_step, size_enclosure, o_soa);
		}

		collision_check(o_soa);
	}

	double toc = omp_get_wtime();
	printf("tiempo de ejecucion: %f\n", toc-tic);
}
