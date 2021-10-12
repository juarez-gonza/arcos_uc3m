#pragma once

struct vec {
	double x;
	double y;
	double z;
};

struct obj {
	struct vec pos;
	struct vec vel;
	struct vec fgv;
	double m;
	/* true cuando el objeto exsite, false cuando no existe */
	bool estado;
};

#define obj_exists(o_ptr) (o_ptr)->estado

int init_obj(struct obj *o);
void destroy_obj(struct obj *o);
void merge_obj(struct obj *o_ip, struct obj *o_jp);

struct obj_list {
	struct obj *list;
	unsigned int size;
};

int init_obj_list(struct obj_list *o_list, unsigned int size,
		unsigned int random_seed, double upperbound);
void destroy_obj_list(struct obj_list *o_list);
