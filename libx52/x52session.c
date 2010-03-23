#include "x52session.h"
#include "x52interface.h"

/* callbacks */

int32_t
x52s_enable()
{
	int32_t res = x52i_open_device();
	if (res == -1)
		return 0;
	return 1;
}

void
x52s_disable()
{
	x52i_close_device();
}

