#include "x52session.h"
#include "x52interface.h"

/* callbacks */

int32_t
x52s_enable(void(*fn)(void))
{
	int32_t res = x52i_open_device();
	if (res == -1)
		return 0;
	if (fn) fn();
	return 1;
}

void
x52s_disable(void(*fn)(void))
{
	if (fn) fn();
	x52i_close_device();
}

