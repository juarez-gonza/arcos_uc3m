#pragma once

#include <vector>
#include <random>

struct obj {
	double x, y, z;
	double m;
	double fx, fy, fz;
	/* vz y vx entran en otra linea de cache */
	double vx, vy, vz;
}__attribute__((aligned(8))); /* aligned 8 es el comportamiento default */

void obj_copy_into(struct obj &o_into, struct obj &o_from);

void merge_obj(struct obj &o_i, struct obj &o_j);

class aos : public std::vector<struct obj> {
private:
	size_t len;
public:
	explicit aos(size_t size, unsigned int random_seed,
			double upperbound) :
		std::vector<struct obj>(size),
		len{size}
	{
		std::mt19937_64 gen(random_seed);
		std::uniform_real_distribution<> uniform(0, upperbound);
		std::normal_distribution<> normal(1e+21, 1e+15);

		for (unsigned long i = 0; i < len; ++i) {

			/* forma moralmente sucia para extender constructor de std::vector
			 * y asignar valores a los elementos en el mismo.
			 * El compilador puede no generar muy buen codigo
			 * para esto pero se corre una sola vez en todo el programa y
			 * no quiero tener un puntero a vector, si no extenderlo
			 */
			(*this)[i].x = uniform(gen);
			(*this)[i].y = uniform(gen);
			(*this)[i].z = uniform(gen);
			(*this)[i].m = normal(gen);

			(*this)[i].fx = 0;
			(*this)[i].fy = 0;
			(*this)[i].fz = 0;

			(*this)[i].vx = 0;
			(*this)[i].vy = 0;
			(*this)[i].vz = 0;
		}
	}

	void set_len(size_t new_len)
	{
		len = new_len;
	}

	size_t get_len()
	{
		return len;
	}
};
