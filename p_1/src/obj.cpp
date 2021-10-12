#include "obj.h"
#include <stdlib.h>
#include <random>

#include <iostream>

int init_obj(struct obj *op)
{
	op->estado = true;
	/* 0-initialize */
	op->vel.x = 0;
	op->vel.y = 0;
	op->vel.z = 0;

	op->fgv.x = 0;
	op->fgv.y = 0;
	op->fgv.z = 0;

	return 0;
}

#ifndef NO_PTR_OBJ
void destroy_obj(struct obj *op)
{
}
#endif

void merge_obj(struct obj *o_ip, struct obj *o_jp)
{
	o_ip->m += o_jp->m;
	o_ip->vel.x += o_jp->vel.x;
	o_ip->vel.y += o_jp->vel.y;
	o_ip->vel.z += o_jp->vel.z;
	o_jp->estado = false;
	/* no llama al destructor de o_jp, de eso se encarga la funcion
	 * donde se defina o_jp o el contenedor que lo contenga.
	 */

	/*
	std::cout << "merge " << o_jp << " into " << o_ip << '\n';
	*/
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
#ifndef NO_PTR_OBJ
		if (init_obj(op))
			goto loop_error;
#else
		init_obj(op);
#endif
		op->pos.x = uniform(gen);
		op->pos.y = uniform(gen);
		op->pos.z = uniform(gen);
		op->m = normal(gen);
		/*
		std::cout << "creando op:\t" << op << "\n\tx: " << op->pos.x
			<< "\n\ty: " << op->pos.y
			<< "\n\tz: " << op->pos.z
			<< "\n\tm: " << op->m << "\n";
		*/
	}

	return 0;
#ifndef NO_PTR_OBJ
loop_error:
	/* on loop_error, destroy correctly allocated objects
	 * in the above lines. careful with free after free.
	 * if o_listp->list has the loop_error, if statements testing for NULL
	 * in object field will avoid freeing uninitialized data.
	 */
	for (; op >= o_listp->list; --op) {
		destroy_obj(op);
	}
	free(o_listp->list);
	o_listp->list = NULL;
#endif
list_error:
	return 1;
}

void destroy_obj_list(struct obj_list *o_listp)
{
	struct obj *op;

	if (o_listp == NULL || o_listp->list == NULL)
		return;

	/* solo aquí se pueden liberar recursos de objetos pertenecientes
	 * al container, por lo que el container usado correctamente
	 * no necesitaria llamar a obj_exists(op)
	 */
#ifndef NO_PTR_OBJ
	for (op = o_listp->list; op != o_listp->list + o_listp->size; ++op)
		if (op != NULL)
			destroy_obj(op);
#endif
	free(o_listp->list);
	o_listp->list = NULL;
}
