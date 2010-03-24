#include <stdio.h>

void* control_init(void)
{
	// find datarefs, allocate other resources here
	return 0;
}

void control_update(void* arg)
{
	// retrieve registered datarefs, compare_and_swap, update joystick
}

void control_end(void* arg)
{
	// clean up allocated resources here
}
