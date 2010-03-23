#include "x52wrapper.h"
#include "x52device.h"
#include "usb/usb.h"

enum x52d_id
{
	x52d_saitek	= 0x06a3,
	x52d_x52_1	= 0x0255,
	x52d_x52_2	= 0x075c,
	x52d_x52pro	= 0x0762,
	x52d_yoke	= 0x0bac,
};

struct x52d_single_t {
	struct usb_dev_handle*	handle;
	const char*		name;
	enum x52d_id		id;
};

struct x52d_t {
	struct x52d_single_t*	devices;
	size_t			n_devices;
	size_t			current;
};

static inline void
_x52d_alloc_new(struct x52d_t* x52d, enum x52d_id id, struct usb_device* dev,
const char* name)
{
	struct usb_dev_handle* t;

	t = usb_open(dev);
	if (!t)
		x52w_die(__func__);

	x52d->n_devices++;
	x52d->devices = x52w_realloc(x52d->devices,
			x52d->n_devices*sizeof(struct x52d_single_t));
	x52d->devices[x52d->n_devices-1].id = id;
	x52d->devices[x52d->n_devices-1].name = name;
	x52d->devices[x52d->n_devices-1].handle = t;
}

struct x52d_t*
x52d_enumerate(void)
{
	struct usb_bus* bus;
	struct usb_device* dev;
	struct x52d_t* x52d;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	x52d = x52w_calloc(1, sizeof(struct x52d_t));

	for (bus = usb_busses; bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor != x52d_saitek) continue;
			switch (dev->descriptor.idProduct) {
			case x52d_x52_1:
				_x52d_alloc_new(x52d, x52d_x52_1, dev,
					"Saitek X52 Flight Control System");
				break;
			case x52d_x52_2:
				_x52d_alloc_new(x52d, x52d_x52_2, dev,
					"Saitek X52 Flight Control System");
				break;
			case x52d_x52pro:
				_x52d_alloc_new(x52d, x52d_x52pro, dev,
					"Saitek X52 Pro Flight Control System");
				break;
			case x52d_yoke:
				_x52d_alloc_new(x52d, x52d_yoke, dev,
					"Saitek Pro Flight Yoke System");
				break;
			}
		}
        }

	return x52d;
}

void
x52d_close(struct x52d_t* x52d)
{
	size_t i;
	if (x52d->n_devices) {
		for (i = 0; i < x52d->n_devices; ++i)
			usb_close(x52d->devices[i].handle);
		free(x52d->devices);
	}
	free(x52d);
}

size_t
x52d_ndevices(struct x52d_t* x52d)
{
	return x52d->n_devices;
}

int32_t
x52d_set_current(struct x52d_t* x52d, size_t device)
{
	if (device >= x52d->n_devices)
		return -1;
	x52d->current = device;
	return 0;
}

int32_t
x52d_control(struct x52d_t* x52d, uint16_t val, uint8_t idx)
{
	return usb_control_msg(x52d->devices[x52d->current].handle,
		USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
		0x91, val, idx, 0, 0, 50);
}

