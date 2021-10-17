#include "obj.hpp"

static inline void _merge_obj(struct obj_aos &o_aos,
		unsigned int i, unsigned int j)
{
	o_aos.m[i] += o_aos.m[j];
	o_aos.vx[i] += o_aos.vx[j];
	o_aos.vy[i] += o_aos.vy[j];
	o_aos.vz[i] += o_aos.vz[j];
}

void merge_obj(struct obj_aos &o_aos,
		unsigned int i, unsigned int j)
{
	_merge_obj(o_aos, i, j);

	o_aos.x[j] = std::move(o_aos.x.back());
	o_aos.x.pop_back();
	o_aos.y[j] = std::move(o_aos.y.back());
	o_aos.y.pop_back();
	o_aos.z[j] = std::move(o_aos.z.back());
	o_aos.z.pop_back();

	o_aos.m[j] = std::move(o_aos.m.back());
	o_aos.m.pop_back();

	o_aos.vx[j] = std::move(o_aos.vx.back());
	o_aos.vx.pop_back();
	o_aos.vy[j] = std::move(o_aos.vy.back());
	o_aos.vy.pop_back();
	o_aos.vz[j] = std::move(o_aos.vz.back());
	o_aos.vz.pop_back();

	o_aos.fx[j] = std::move(o_aos.fx.back());
	o_aos.fx.pop_back();
	o_aos.fy[j] = std::move(o_aos.fy.back());
	o_aos.fy.pop_back();
	o_aos.fz[j] = std::move(o_aos.fz.back());
	o_aos.fz.pop_back();
}
