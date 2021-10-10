#include "obj.h"
#include <stdlib.h>
#include <random>

#include <iostream>

int init_obj(struct obj *o)
{
	/* 0-initialize */
	o->pos = (struct vec *)calloc(sizeof(vec), 1);
	if (o->pos == NULL)
		goto error_pos;

	o->vel = (struct vec *)calloc(sizeof(vec), 1);
	if (o->vel == NULL)
		goto error_vel;

	o->fgv = (struct vec *)calloc(sizeof(vec), 1);
	if (o->fgv == NULL)
		goto error_fgv;

	return 0;
error_fgv:
	free(o->vel);
error_vel:
	free(o->pos);
error_pos:
	return 1;
}

void destroy_obj(struct obj *o)
{
	free(o->pos);
	free(o->vel);
	free(o->fgv);
}

int init_obj_list(struct obj_list *o_listp, unsigned int size,
		unsigned int random_seed, double upperbound)
{
	struct obj *o;
	std::mt19937_64 gen(random_seed);
	std::uniform_real_distribution<> uniform(0, upperbound);
	std::normal_distribution<> normal(10e+21, 10e+15);

	o_listp->size = size;
	o_listp->list = (struct obj *)malloc(sizeof(struct obj) * size);
	if (o_listp->list == NULL)
		goto list_error;

	for (o = o_listp->list; o != o_listp->list + size; ++o) {
		if (init_obj(o))
			goto loop_error;
		o->pos->x = uniform(gen);
		o->pos->y = uniform(gen);
		o->pos->z = uniform(gen);
		o->m = normal(gen);
		/*
		std::cout << "o:\t" << o << "\n\tx: " << o->pos->x
			<< "\n\ty: " << o->pos->y
			<< "\n\tz: " << o->pos->z
			<< "\n\tm: " << o->m << "\n";
		*/
	}

	return 0;
loop_error:
	/* on loop_error, destroy correctly allocated objects
	 * in the above lines. careful with free after free
	 */
	for (--o; o != o_listp->list; --o)
		destroy_obj(o);
	destroy_obj(o);
	free(o_listp->list);
list_error:
	return 1;
}

void destroy_obj_list(struct obj_list *o_listp)
{
	struct obj *o;

	for (o = o_listp->list; o != o_listp->list + o_listp->size; ++o)
		if (o != NULL)
			destroy_obj(o);

	free(o_listp->list);
}
