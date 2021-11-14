#pragma once

#include <stdlib.h>
#include <cstring>
#include <random>

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

/* chequear inline */
static inline bool obj_marked(size_t idx, struct soa &o_soa)
{
	return o_soa.m[idx] <= 0;
}

void obj_copy_into(size_t into_idx, size_t from_idx, struct soa &o_soa);

void merge_obj(size_t i, size_t j, struct soa &o_soa);
