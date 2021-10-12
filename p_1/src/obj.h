#pragma once

#define NO_PTR_OBJ

struct vec {
	double x;
	double y;
	double z;
}__attribute__((aligned(8)));

struct obj {
	struct vec pos;
	struct vec vel;
	struct vec fgv;
	double m;
	bool estado;
	/* true cuando el objeto existe, false cuando no existe */
	/* facilita el merge de objetos sin necesitar copiar el array
	 * para quitar un objeto ni recurrir a una linked list */
}__attribute__((aligned(8)));

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
