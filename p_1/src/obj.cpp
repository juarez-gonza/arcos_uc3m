#include "obj.h"
#include <stdlib.h>
#include <random>

#include <iostream>

int init_obj(struct obj *op)
{
	/* 0-initialize */
	op->pos = (struct vec *)calloc(sizeof(vec), 1);
	if (op->pos == NULL)
		goto error_pos;

	op->vel = (struct vec *)calloc(sizeof(vec), 1);
	if (op->vel == NULL)
		goto error_vel;

	op->fgv = (struct vec *)calloc(sizeof(vec), 1);
	if (op->fgv == NULL)
		goto error_fgv;

	return 0;
error_fgv:
	free(op->vel);
error_vel:
	free(op->pos);
error_pos:
	return 1;
}

void destroy_obj(struct obj *op)
{
	free(op->pos);
	free(op->vel);
	free(op->fgv);
}

int init_obj_list(struct obj_list *o_listp, unsigned int size,
		unsigned int random_seed, double upperbound)
{
	struct obj *op;
	std::mt19937_64 gen(random_seed);
	std::uniform_real_distribution<> uniform(0, upperbound);
	std::normal_distribution<> normal(10e+21, 10e+15);

	o_listp->size = size;
	o_listp->list = (struct obj *)malloc(sizeof(struct obj) * size);
	if (o_listp->list == NULL)
		goto list_error;

	for (op = o_listp->list; op != o_listp->list + size; ++op) {
		if (init_obj(op))
			goto loop_error;
		op->pos->x = uniform(gen);
		op->pos->y = uniform(gen);
		op->pos->z = uniform(gen);
		op->m = normal(gen);
		/*
		std::cout << "op:\t" << op << "\n\tx: " << op->pos->x
			<< "\n\ty: " << op->pos->y
			<< "\n\tz: " << op->pos->z
			<< "\n\tm: " << op->m << "\n";
		*/
	}

	return 0;
loop_error:
	/* on loop_error, destroy correctly allocated objects
	 * in the above lines. careful with free after free
	 */
	for (--op; op != o_listp->list; --op)
		destroy_obj(op);
	destroy_obj(op);
	free(o_listp->list);
list_error:
	return 1;
}

void destroy_obj_list(struct obj_list *o_listp)
{
	struct obj *op;

	for (op = o_listp->list; op != o_listp->list + o_listp->size; ++op)
		if (op != NULL)
			destroy_obj(op);

	free(o_listp->list);
}
