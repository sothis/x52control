#ifndef _X52WRAPPER_H
#define _X52WRAPPER_H

#include <stdlib.h>
#include <stdio.h>

static inline void x52w_die(const char* msg)
{
	printf("x52control: error in %s(). exiting.\n", msg);
	exit(1);
}

static inline void* x52w_malloc(size_t s)
{
	void* t;

	t = malloc(s);
	if (!t)
		x52w_die(__func__);
	return t;
}

static inline void* x52w_calloc(size_t n, size_t s)
{
	void* t;

	t = calloc(n, s);
	if (!t)
		x52w_die(__func__);
	return t;
}

static inline void* x52w_realloc(void*p, size_t s)
{
	void* t;

	t = realloc(p, s);
	if (!t)
		x52w_die(__func__);
	return t;
}

#endif /* _X52WRAPPER_H */

