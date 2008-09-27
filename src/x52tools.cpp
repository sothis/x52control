#include <stdio.h>
#include <stdarg.h>
#include "x52tools.h"

x52tools_t::x52tools_t(void) : verbose(false)
{
}

void x52tools_t::debug_out(int type, const char* msg, ...)
{
    if ((type == info) && (!verbose)) return;
    va_list ap;
    va_start(ap, msg);
    switch (type)
    {
    case info:
        fprintf(stderr, "\033[0;32m[x52control]\033[0m: ");
        break;
    case warn:
        fprintf(stderr, "\033[0;34m[x52control]\033[0m: ");
        break;
    case err:
    default:
        fprintf(stderr, "\033[0;31m[x52control]\033[0m: ");
        break;
    }
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    fflush(stderr);
}

void x52tools_t::debug_out(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    fprintf(stderr, "[x52control]: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    fflush(stderr);
}
