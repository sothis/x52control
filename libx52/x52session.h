#ifndef _X52SESSION_H
#define _X52SESSION_H

#include <stdint.h>

/* callbacks */

extern int32_t
x52s_enable(void(*fn)(void));

extern void
x52s_disable(void(*fn)(void));

#endif /* _X52SESSION_H */

