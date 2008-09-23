#include <stdio.h>
#include <stdarg.h>
#include "x52tools.h"

x52tools_t::x52tools_t(void) : verbose(false)
{
}

void x52tools_t::debug_out(int type, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	switch (type)
	{
		case info:
			if (verbose)
			{
				printf("\033[0;32m[x52control]\033[0m: ");
				vprintf(msg, ap);
				printf("\n");
			}
			break;
		case warn:
			printf("\033[0;34m[x52control]\033[0m: ");
			vprintf(msg, ap);
			printf("\n");
			break;
		case err:
		default:
			printf("\033[0;31m[x52control]\033[0m: ");
			vprintf(msg, ap);
			printf("\n");
			break;
	}
	va_end(ap);
}

void x52tools_t::debug_out(const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	printf("[x52control]: ");
	vprintf(msg, ap);
	printf("\n");
	va_end(ap);
}

