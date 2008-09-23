#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "usb/usb.h"
#include "output.h"
#include "XPLM/XPLMProcessing.h"

#define STANDARD_MSG		"   Saitek X52\n     Flight\n Control System"
#define STANDARD_MSG_PRO	" Saitek X52 Pro\n     Flight\n Control System"

enum devices_e
{
	x52_standard_device		= 0x0255,
	x52_standard_device2	= 0x075C,
	x52_pro_device			= 0x0762,
	yoke_device				= 0x0BAC
};

enum dbgtype_e
{
	info,
	warn,
	err
};

x52out_t::x52out_t(void) : verbose(false)
{
	usb_bus* bus				= 0;
	struct usb_device* joydev	= 0;
	struct usb_device* dev		= 0;
	usb_device_descriptor* dsc	= 0;

	pthread_mutex_init(&param_mutex, NULL);

	debug_out(warn, "searching for joystick");
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
}

x52out_t::~x52out_t(void)
{
	debug_out(warn, "start unloading");
	usb_close(a_usbhdl);
	debug_out(warn, "unloading succeeded");
}

void x52out_t::debug_out(int type, const char* msg, ...)
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

void x52out_t::refresh(const out_param_t* param)
{
	pthread_mutex_lock(&param_mutex);
	memcpy(&current_param, const_cast<out_param_t*>(param), sizeof(out_param_t));
	pthread_mutex_unlock(&param_mutex);
	XPLMSetFlightLoopCallbackInterval(x52out_t::update, -1.0f, 1, this);
}

/* static member, x-plane calls this one */
float x52out_t::update(float elapsed_lastcall, float elapsed_lastloop, int n_loop, void* arg)
{
	x52out_t* me = reinterpret_cast<x52out_t*>(arg);
	pthread_mutex_lock(&me->parameter_mutex());
	float res = me->current_state().next_call;
	me->debug_out(info, "updating joystick state (loop %d)", n_loop);
	me->print(me->current_state().text);
	pthread_mutex_unlock(&me->parameter_mutex());
	return res;
}

const out_param_t& x52out_t::current_state(void)
{
	return current_param;
}

pthread_mutex_t& x52out_t::parameter_mutex(void)
{
	return param_mutex;
}

void x52out_t::set_verbose(bool verb)
{
	verbose = verb;
}

void x52out_t::print(const char* text)
{
	int n_lf = 0;
	if (!text || product == yoke_device) return;
	if (!strlen(text))
		clear();
	char* temp = new char[strlen(text)];
	if (!temp) return;
	strcpy(temp, text);

	char* token = strtok(temp, "\n");
    while(token && (n_lf < 3))
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
			debug_out(err, reason);
		}
		n_lf++;
		token = strtok(0, "\n");
    }
	delete[] temp;
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

/* private members */

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
        res =	usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,
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

	res =	usb_control_msg(a_usbhdl, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x91,
							0x00, line_clearctl[line], 0, 0, 100);
	if (res < 0)
		throw "could not clear textline";
}

