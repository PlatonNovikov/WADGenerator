#ifndef VECTOR_H
#define VECTOR_H

/*
A simple vector lib
Honestly, i have no idea why do i write it from scratch
It's slow and pointless
i guess just for fun
*/

#include <stdlib.h>
#include <stdbool.h>

typedef struct s_vec
{
	void	**data;
	size_t	size;
	size_t	capacity;
} t_vec;

t_vec	*vec_init(void);
void	vec_append(t_vec *vec, void *ptr);
bool	vec_ord_rm(t_vec *vec, void *ptr);
void	*vec_get(t_vec *vec, size_t index);
void	vec_free(t_vec *vec);

#endif
