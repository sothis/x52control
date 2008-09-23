#include <stdio.h>
#include <string.h>
#include "usb/usb.h"
#include "usbifce.h"

struct x52
{
    usb_dev_handle *hdl;
    enum x52_type type;
unsigned feat_mfd:
    1;
unsigned feat_led:
    1;
unsigned feat_sec:
    1;
unsigned debug:
    1;
};

#define X52PRO_REQUEST 0x91

#define X52PRO_CLEAR1 0xd9
#define X52PRO_CLEAR2 0xda
#define X52PRO_CLEAR3 0xdc
#define X52PRO_WRITE1 0xd1
#define X52PRO_WRITE2 0xd2
#define X52PRO_WRITE3 0xd4
#define X52PRO_SETLED 0xb8
#define X52PRO_MFDBRI 0xb1
#define X52PRO_LEDBRI 0xb2

#define X52PRO_TIME   0xc0
#define X52PRO_OFFS2  0xc1
#define X52PRO_OFFS3  0xc2
#define X52PRO_DATE   0xc4
#define X52PRO_YEAR   0xc8

#define YOKE_SECOND   0xca

int write_idx[3] =
{
    X52PRO_WRITE1,
    X52PRO_WRITE2,
    X52PRO_WRITE3,
};

int clear_idx[3] =
{
    X52PRO_CLEAR1,
    X52PRO_CLEAR2,
    X52PRO_CLEAR3,
};

#define x52printf fprintf

int x52_cleartext(struct x52 *x52, int line)
{
    int r;
    if (!x52->feat_mfd)
    {
        x52printf(stderr, "settext not supported\n");
        return -3;
    }
    if (line > 2) return -1;
    r = usb_control_msg(x52->hdl,
                        USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                        X52PRO_REQUEST, 0x00, clear_idx[line], NULL, 0, 100);
    if (r < 0)
    {
        x52printf(stderr, "x52_settext failed at clear command (%s)\n",
                  usb_strerror());
        return -2;
    }
    return 0;
}

int x52_clearall(struct x52 *x52)
{
    int r;
    for (r=0; r<3;++r)
        x52_cleartext(x52, r);
    return 0;
}

int x52_settext(struct x52 *x52, int line, char *text, int length)
{
    int r;
    if (!x52->feat_mfd)
    {
        x52printf(stderr, "settext not supported\n");
        return -3;
    }
    if (line > 2) return -1;
	usb_control_msg(x52->hdl,
                    USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                    X52PRO_REQUEST, 0x00, clear_idx[line], NULL, 0, 100);
    while (length >= 1)
    {
        int chars;
        if (length == 1) chars = (' ' << 8) + *text;
        else chars = *(unsigned short*) text;
        r = usb_control_msg(x52->hdl,
                            USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                            X52PRO_REQUEST, chars, write_idx[line], NULL, 0, 100);
        if (r<0)
        {
            x52printf(stderr, "x52_settext failed at write %d (%s)\n",
                      length, usb_strerror());
            return -2;
        }
        length -= 2;
        text += 2;
    }
    return 0;
}

int x52_setbri(struct x52 *x52, int mfd, int brightness)
{
    int r;
    if (!x52->feat_mfd)
    {
        x52printf(stderr, "setbri not supported\n");
        return -3;
    }
    r = usb_control_msg(x52->hdl,
                        USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                        X52PRO_REQUEST, brightness, mfd ? X52PRO_MFDBRI : X52PRO_LEDBRI,
                        NULL, 0, 100);
    if (r < 0)
    {
        x52printf(stderr, "x52_setbri failed (%s)\n", usb_strerror());
        return -2;
    }

    return 0;
}

int x52_setled(struct x52 *x52, int led, int on)
{
    int r;
    if (!x52->feat_led)
    {
        x52printf(stderr, "setled not supported\n");
        return -3;
    }

    r = usb_control_msg(x52->hdl,
                        USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                        X52PRO_REQUEST, on | (led<<8), X52PRO_SETLED,
                        NULL, 0, 100);
    if (r < 0)
    {
        x52printf(stderr, "x52_setled failed (%s)\n", usb_strerror());
        return -2;
    }

    return 0;
}

int x52_settime(struct x52 *x52, int h24, int hour, int minute)
{
    int r;
    r = usb_control_msg(x52->hdl,
                        USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                        X52PRO_REQUEST, minute | (hour<<8) | (h24?0x8000:0), X52PRO_TIME,
                        NULL, 0, 100);
    if (r < 0)
    {
        x52printf(stderr, "x52_settime failed (%s)\n", usb_strerror());
        return -2;
    }
    return 0;
}

int x52_setoffs(struct x52 *x52, int idx, int h24, int inv, int offset)
{

    int r;
    r = usb_control_msg(x52->hdl,
                        USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT, X52PRO_REQUEST,
                        offset | (inv?1024:0) | (h24?0x8000:0), idx?X52PRO_OFFS3:X52PRO_OFFS2,
                        NULL, 0, 100);
    if (r < 0)
    {
        x52printf(stderr, "x52_settime failed (%s)\n", usb_strerror());
        return -2;
    }

    return 0;
}

int x52_setsecond(struct x52 *x52, int second)
{
    int r;
    if (!x52->feat_sec)
    {
        x52printf(stderr, "setsecond not supported\n");
        return -3;
    }
    r = usb_control_msg(x52->hdl,
                        USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                        X52PRO_REQUEST, second<<8, YOKE_SECOND,
                        NULL, 0, 100);
    if (r < 0)
    {
        x52printf(stderr, "x52_setsecond failed (%s)\n", usb_strerror());
        return -2;
    }

    return 0;
}

int x52_setdate(struct x52 *x52, int year, int month, int day)
{
    int r;
    if (!x52->feat_mfd)
    {
        x52printf(stderr, "setdate not supported\n");
        return -3;
    }
    r = usb_control_msg(x52->hdl,
                        USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                        X52PRO_REQUEST, day | (month<<8), X52PRO_DATE,
                        NULL, 0, 100);
    if (r < 0)
    {
        x52printf(stderr, "x52_setdate failed (%s)\n", usb_strerror());
        return -2;
    }
    r = usb_control_msg(x52->hdl,
                        USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                        X52PRO_REQUEST, year, X52PRO_YEAR,
                        NULL, 0, 100);
    if (r < 0)
    {
        x52printf(stderr, "x52_setdate failed for year (%s)\n", usb_strerror());
        return -2;
    }
    return 0;
}

int x52_custom(struct x52 *x52, int index, int value)
{

    int r = usb_control_msg(x52->hdl,
                            USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,
                            X52PRO_REQUEST, value, index, NULL, 0, 100);
    if (r < 0)
    {
        x52printf(stderr, "x52_settext failed at clear command (%s)\n",
                  usb_strerror());
        return -2;
    }

    return 0;
}

#define VENDOR_SAITEK 0x6a3
#define PRODUCT_X52_0 0x255
#define PRODUCT_X52_1 0x75c
#define PRODUCT_X52PRO 0x762
#define PRODUCT_YOKE 0xbac

struct x52* x52_init(void)
{
    struct x52 x52, *x52p;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    memset(&x52, 0, sizeof(x52));

    struct usb_bus *bus;
    struct usb_device *joydev = NULL;
    for (bus = usb_busses; bus; bus = bus->next)
    {
        struct usb_device *dev;
        for (dev = bus->devices; dev; dev = dev->next)
        {
            struct usb_device_descriptor *dsc = &dev->descriptor;
            if (dsc->idVendor != VENDOR_SAITEK) continue;
            switch (dsc->idProduct)
            {
            case PRODUCT_X52_0:
            case PRODUCT_X52_1:
                x52.feat_mfd = 1;
                x52.type = DEV_X52;
                joydev = dev;
                break;
            case PRODUCT_X52PRO:
                x52.feat_mfd = 1;
                x52.feat_led = 1;
                x52.type = DEV_X52PRO;
                joydev = dev;
                break;
            case PRODUCT_YOKE:
                x52.feat_sec = 1;
                x52.type = DEV_YOKE;
                joydev = dev;
                break;
            }
            if (joydev) break;
        }
        if (joydev) break;
    }
    if (!joydev)
    {
        fprintf(stderr, "joystick not found\n");
        return NULL;
    }
    x52.hdl = usb_open(joydev);
    if (x52.hdl==NULL)
    {
        fprintf(stderr, "joystick open failed\n");
        return NULL;
    }
    x52p = malloc(sizeof(*x52p));
    *x52p = x52;
    return x52p;
}

enum x52_type x52_gettype(struct x52* hdl)
{
    return hdl->type;
}

void x52_close(struct x52* x52)
{
    int r;
    r = usb_close(x52->hdl);
    free(x52);
}

void x52_debug(struct x52* x52, int debug)
{
    x52->debug = debug;
}
