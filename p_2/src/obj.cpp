#include "obj.hpp"

void obj_copy_into(size_t into_idx, size_t from_idx, struct soa &o_soa)
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

void merge_obj(size_t i, size_t j, struct soa &o_soa)
{
	o_soa.m[i] += o_soa.m[j];
	o_soa.vx[i] += o_soa.vx[j];
	o_soa.vy[i] += o_soa.vy[j];
	o_soa.vz[i] += o_soa.vz[j];
}
