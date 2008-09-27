#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "usb/usb.h"
#include "x52out.h"

extern const char* version;

#define STANDARD_MSG  "   Saitek X52\n     Flight\n Control System"
#define STANDARD_MSG_PRO " Saitek X52 Pro\n     Flight\n Control System"
#define WELCOME_MSG   "   x52control\n \n      ver %s", version

enum devices_e
{
    x52_standard_device  = 0x0255,
    x52_standard_device2 = 0x075C,
    x52_pro_device   = 0x0762,
    yoke_device    = 0x0BAC
};

x52out_t::x52out_t(void)
{
    usb_bus* bus    = 0;
    struct usb_device* joydev = 0;
    struct usb_device* dev  = 0;
    usb_device_descriptor* dsc = 0;

    debug_out(info, "searching for joystick");
    usb_init();
    usb_find_busses();
    usb_find_devices();

    for (bus = usb_busses; bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            dsc = &dev->descriptor;
            if (dsc->idVendor != 0x06A3) continue;
            switch (dsc->idProduct)
            {
            case x52_standard_device:
                product = x52_standard_device;
                debug_out(warn, "Saitek X52 Flight Control System found (Product ID: 0x%.4X)", product);
                joydev = dev;
                break;
            case x52_standard_device2:
                product = x52_standard_device2;
                debug_out(warn, "Saitek X52 Flight Control System found (Product ID: 0x%.4X)", product);
                joydev = dev;
                break;
            case x52_pro_device:
                product = x52_pro_device;
                debug_out(warn, "Saitek X52 Pro Flight Control System found (Product ID: 0x%.4X)", product);
                joydev = dev;
                break;
            case yoke_device:
                product = yoke_device;
                debug_out(warn, "Saitek Pro Flight Yoke System found (Product ID: 0x%.4X)", product);
                joydev = dev;
                break;
            }
            if (joydev) break;
        }
        if (joydev) break;
    }
    if (!joydev) throw "no compatible joystick found";

    a_usbhdl = usb_open(joydev);
    if (!a_usbhdl) throw "could not open joystick";
    display_brightness(0x7F);
    led_brightness(0x7F);
    print(WELCOME_MSG);
}

x52out_t::~x52out_t(void)
{
    switch (product)
    {
    case x52_standard_device:
    case x52_standard_device2:
        print(STANDARD_MSG);
        break;
    case x52_pro_device:
        print(STANDARD_MSG_PRO);
        break;
    case yoke_device:
    default:
        print("");
        break;
    }
    time(true, 0, 0);
    date(0, 0, 0);
    usb_close(a_usbhdl);
    debug_out(info, "joystick disconnected");
}

void x52out_t::display_brightness(char brightness)
{
    try
    {
        setbrightness(true, brightness);
    }
    catch (const char* reason)
    {
        debug_out(err, "%s of multifunction display", reason);
    }
}

void x52out_t::led_brightness(char brightness)
{
    try
    {
        setbrightness(false, brightness);
    }
    catch (const char* reason)
    {
        debug_out(err, "%s of LED's", reason);
    }
}

void x52out_t::set_textdata(const char* text)
{
    if (!text) return;
    a_textdata.assign(text);
}

void x52out_t::print()
{
    print(a_textdata.c_str());
}

void x52out_t::print(const char* t, ...)
{
    int n_lf = 0;
    // the display only supports 48 characters, we allow 2 additional newline
    // characters and a terminating null byte, any additional characters are discarded
    char text[51] = {};
    if (!t || product == yoke_device) return;
    //clear();
    if (!strlen(t)) return;
    va_list ap;
    va_start(ap, t);
    vsnprintf(text, 51, t, ap);
    va_end(ap);

    char* token = strtok(text, "\n");
    while (token && (n_lf < 3))
    {
        char line[17] = {};
        strncpy(line, token, 16);
        line[16] = 0;
        try
        {
            settext(n_lf, line, 16);
        }
        catch (const char* reason)
        {
            debug_out(err, "%s (%s)", reason, line);
        }
        n_lf++;
        token = strtok(0, "\n");
    }
}

void x52out_t::clear(void)
{
    if (product == yoke_device) return;
    try
    {
        cleartext(0);
        cleartext(1);
        cleartext(2);
    }
    catch (const char* reason)
    {
        debug_out(err, reason);
    }
}

void x52out_t::time(bool h24, char hour, char minute)
{
    try
    {
        settime(h24, hour, minute);
    }
    catch (const char* reason)
    {
        debug_out(err, "%s", reason);
    }
}

void x52out_t::date(int year, int month, int day)
{
    if (product == yoke_device) return;
    try
    {
        setdate(year, month, day);
    }
    catch (const char* reason)
    {
        debug_out(err, "%s", reason);
    }
}


/* private members */

void x52out_t::setbrightness(bool mfd, char brightness)
{
    int res = 0;

    res = usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,
                          brightness, mfd?0xB1:0xB2, 0, 0, 100);
    if (res < 0)
        throw "could not set brightness";
}

void x52out_t::settext(int line, const char *text, int length)
{
    int res = 0;
    if (!text) return;
    unsigned char line_writectl[3] = {0xD1, 0xD2, 0xD4};
    cleartext(line);
    while (length >= 1)
    {
        unsigned short charpair;
        if (length == 1) charpair = (0 << 8) + *text;
        else charpair = *(unsigned short*) text;
        res = usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,
                              charpair, line_writectl[line], 0, 0, 100);
        if (res < 0)
            throw "could not set textline";
        length -= 2;
        text += 2;
    }
}

void x52out_t::cleartext(int line)
{
    int res = 0;
    unsigned char line_clearctl[3] = {0xD9, 0xDA, 0xDC};

    res = usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,
                          0x00, line_clearctl[line], 0, 0, 100);
    if (res < 0)
        throw "could not clear textline";
}

void x52out_t::settime(int h24, int hour, int minute)
{
    int res = 0;
    res = usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,
                          minute | (hour<<8) | (h24?0x8000:0), 0xC0, 0, 0, 100);
    if (res < 0)
        throw "could not set time";
}

void x52out_t::setdate(int year, int month, int day)
{
    int res = 0;
    res = usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,
                          day | (month<<8), 0xC4, 0, 0, 100);
    if (res < 0)
        throw "could not set day and month";
    res = usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,
                          year, 0xC8, 0, 0, 100);
    if (res < 0)
        throw "could not set year";
}
