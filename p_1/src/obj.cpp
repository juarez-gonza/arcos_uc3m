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
	obj_delete(o_aos, j);
}
