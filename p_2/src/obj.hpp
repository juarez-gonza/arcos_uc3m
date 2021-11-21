#pragma once

//#include <stdlib.h>
#include <random>
#include <vector>

#define SIMD_ALIGNMENT 64 /* sirve para SSE, AVX, y AVX512 */
#define ALIGNMENT SIMD_ALIGNMENT
#define _align(_num) ((_num) + (ALIGNMENT-1)) & ~(ALIGNMENT-1)

struct soa {

	size_t len;

	std::vector<double> x, y, z;
	std::vector<double> m;
	std::vector<double> fx, fy, fz;
	std::vector<double> vx, vy, vz;

	soa(size_t ua_len, unsigned int random_seed, double upperbound) :
		x(ua_len), y(ua_len), z(ua_len),
		m(ua_len),
		fx(ua_len, 0), fy(ua_len, 0), fz(ua_len, 0),
		vx(ua_len, 0), vy(ua_len, 0), vz(ua_len, 0)
	{

		std::mt19937_64 gen(random_seed);
		std::uniform_real_distribution<> uniform(0, upperbound);
		std::normal_distribution<> normal(1e+21, 1e+15);

		len = ua_len;
		for (unsigned int i = 0; i < ua_len; ++i) {
			x[i] = uniform(gen);
			y[i] = uniform(gen);
			z[i] = uniform(gen);
			m[i] = normal(gen);
		}
	}
};

/* chequear inline */
static inline bool obj_marked(size_t idx, struct soa &o_soa)
{
	return o_soa.m[idx] <= 0;
}

void obj_copy_into(size_t into_idx, size_t from_idx, struct soa &o_soa);

void merge_obj(size_t i, size_t j, struct soa &o_soa);
