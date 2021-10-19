#include "obj.hpp"

static inline void _merge_obj(struct obj_soa &o_soa,
		unsigned int i, unsigned int j)
{
	o_soa.m[i] += o_soa.m[j];
	o_soa.vx[i] += o_soa.vx[j];
	o_soa.vy[i] += o_soa.vy[j];
	o_soa.vz[i] += o_soa.vz[j];
}

void merge_obj(struct obj_soa &o_soa,
		unsigned int i, unsigned int j)
{
	_merge_obj(o_soa, i, j);
	obj_delete(o_soa, j);
}
