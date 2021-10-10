#pragma once

struct vec {
	double x;
	double y;
	double z;
};

struct obj {
	struct vec *pos;
	struct vec *vel;
	struct vec *fgv;
	double m;
};

int init_obj(struct obj *o);
void destroy_obj(struct obj *o);

struct obj_list {
	struct obj *list;
	unsigned int size;
};

int init_obj_list(struct obj_list *o_list, unsigned int size,
		unsigned int random_seed, double upperbound);
void destroy_obj_list(struct obj_list *o_list);
