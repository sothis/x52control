#ifndef _X52DEVICE_H
#define _X52DEVICE_H

#include <stdint.h>
#include <stddef.h>

typedef struct x52d_t x52d_t;

extern struct x52d_t*
x52d_enumerate(void);

extern void
x52d_close(struct x52d_t* x52d);

extern size_t
x52d_ndevices(struct x52d_t* x52d);

extern int32_t
x52d_set_current(struct x52d_t* x52d, size_t device);

extern int32_t
x52d_control(struct x52d_t* x52d, int32_t val, int32_t idx);

#endif /* _X52DEVICE_H */

