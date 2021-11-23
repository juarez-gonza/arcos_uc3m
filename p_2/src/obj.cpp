#include "obj.hpp"

static inline void _merge_obj(struct obj &o_i, struct obj &o_j)
{
	o_i.m += o_j.m;
	o_i.vx += o_j.vx;
	o_i.vy += o_j.vy;
	o_i.vz += o_j.vz;
}

void merge_obj(struct obj &o_i, struct obj &o_j)
{
	_merge_obj(o_i, o_j);
	//obj_delete(o_j);
}

void obj_copy_into(struct obj &o_into, struct obj &o_from)
{
	o_into.m = o_from.m;
	o_into.x = o_from.x;
	o_into.y = o_from.y;
	o_into.z = o_from.z;

	o_into.vx = o_from.vx;
	o_into.vy = o_from.vy;
	o_into.vz = o_from.vz;

	o_into.fx = o_from.fx;
	o_into.fy = o_from.fy;
	o_into.fz = o_from.fz;
}
