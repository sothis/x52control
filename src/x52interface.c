#include "x52interface.h"
#include "x52device.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define _set(x, y)	(x |= y)
#define _clr(x, y)	(x &= ~y)
#define _get(x, y)	((x & y) != 0)
#define _setall(x)	(x = ~0)
#define _clrall(x)	(x = 0)

enum x52i_control
{
	x52i_ctrl_24hourmode		= 0x8000,
	x52i_ctrl_negativeoffset	= 0x0400,
	x52i_ctrl_brightness		= 0xb0,
/*
	x52i_ctrl_mfd_brightness	= 0xb1,
	x52i_ctrl_led_brightness	= 0xb2,
*/
	x52i_ctrl_setled		= 0xb8,
	x52i_ctrl_time			= 0xc0,
/*
	x52i_ctrl_timeoffset1		= 0xc1,
	x52i_ctrl_timeoffset2		= 0xc2,
*/
	x52i_ctrl_daymonth		= 0xc4,
	x52i_ctrl_year			= 0xc8,
	x52i_ctrl_settext		= 0xd0,
/*
	x52i_ctrl_settext1		= 0xd1,
	x52i_ctrl_settext2		= 0xd2,
	x52i_ctrl_settext3		= 0xd4,
*/
	x52i_ctrl_cleartext		= 0xd8,
/*
	x52i_ctrl_cleartext1		= 0xd9,
	x52i_ctrl_cleartext2		= 0xda,
	x52i_ctrl_cleartext3		= 0xdc,
*/
};

struct x52i_time_t {
	uint8_t		dirty	:1;
	uint8_t		mode24	:1;
	uint8_t		hour	:5;
	uint8_t		minute	:6;
};

struct x52i_zone_t {
	uint8_t		dirty	:2;
	uint8_t		mode24[2];
	int16_t		offset[2];
};

struct x52i_date_t {
	uint8_t		dirty	:1;
	uint8_t		day	:5;
	uint8_t		month	:4;
	uint8_t		year	:4;
};

struct x52i_text_t {
	uint8_t		dirty	:3;
	uint16_t	lines[3][8];
};

struct x52i_led_t {
	uint32_t	dirty	:20;
	uint32_t	state	:20;
};

struct x52i_bright_t {
	uint32_t	dirty	:2;
	uint8_t		value[2];
};

static __thread struct x52i_time_t	x52i_time;
static __thread struct x52i_zone_t	x52i_zone;
static __thread struct x52i_date_t	x52i_date;
static __thread struct x52i_text_t	x52i_text;
static __thread struct x52i_led_t	x52i_led;
static __thread struct x52i_bright_t	x52i_bright;
static __thread struct x52d_t* x52d = 0;


/* state modifiers */

void x52i_set_brightness(enum x52i_brightness dev, uint8_t val)
{
	x52i_bright.value[dev] = val;
	_set(x52i_bright.dirty, (1 << dev));
}

void x52i_set_led(enum x52i_led led, enum x52i_status status)
{
	switch (status) {
	case x52i_status_on:
		_set(x52i_led.state, (1 << led));
		break;
	case x52i_status_off:
		_clr(x52i_led.state, (1 << led));
		break;
	}
	_set(x52i_led.dirty, (1 << led));
}

void x52i_toggle_led(enum x52i_led led)
{
	_get(x52i_led.state, (1 << led))?x52i_set_led(led, x52i_status_off):
	x52i_set_led(led, x52i_status_on);
}

void x52i_set_led_all(enum x52i_status status)
{
	switch (status) {
	case x52i_status_on:
		_setall(x52i_led.state);
		break;
	case x52i_status_off:
		_clrall(x52i_led.state);
		break;
	}
	_setall(x52i_led.dirty);
}

void x52i_set_led_color(enum x52i_cled l, enum x52i_color c,
enum x52i_status status)
{
	switch (c) {
	case x52i_color_red:
		if (status == x52i_status_on) {
			x52i_set_led(l, x52i_status_on);
			x52i_set_led(l+1, x52i_status_off);
		}
		if (status == x52i_status_off) {
			x52i_set_led(l, x52i_status_off);
		}
		break;
	case x52i_color_green:
		if (status == x52i_status_on) {
			x52i_set_led(l, x52i_status_off);
			x52i_set_led(l+1, x52i_status_on);
		}
		if (status == x52i_status_off) {
			x52i_set_led(l+1, x52i_status_off);
		}
		break;
	case x52i_color_amber:
		if (status == x52i_status_on) {
			x52i_set_led(l, x52i_status_on);
			x52i_set_led(l+1, x52i_status_on);
		}
		if (status == x52i_status_off) {
			x52i_set_led(l, x52i_status_off);
			x52i_set_led(l+1, x52i_status_off);
		}
		break;
	}
}

void x52i_set_led_color_all(enum x52i_color c, enum x52i_status status)
{
	for (uint8_t l=x52i_cled_A; l<=x52i_cled_i; l+=2) {
		x52i_set_led_color(l, c, status);
	}
}

void x52i_set_text(enum x52i_line line, const char* txt)
{
	if (!txt) txt = "";
	_set(x52i_text.dirty, (1 << line));
	strncpy((char*)x52i_text.lines[line], txt, sizeof(uint16_t)*8);
	#if defined (__ppc__)
	for (uint8_t charpair = 0; charpair < 8; ++charpair) {
		x52i_text.lines[line][charpair] =
		((x52i_text.lines[line][charpair] >> 8) & 0xff) |
		((x52i_text.lines[line][charpair] & 0xff) << 8);
	}
	#endif /* __ppc__ */
}

void x52i_set_time(uint8_t hour, uint8_t minute, enum x52i_mode mode)
{
	x52i_time.dirty = 1;
	x52i_time.mode24 = mode;
	x52i_time.hour = hour;
	x52i_time.minute = minute;
}

void x52i_set_zone(enum x52i_offset offset, int16_t value, enum x52i_mode mode)
{
	_set(x52i_zone.dirty, (1 << offset));
	x52i_zone.mode24[offset] = mode;
	x52i_zone.offset[offset] = value;
}

void x52i_set_date(uint8_t day, uint8_t month, uint8_t year)
{
	x52i_date.dirty = 1;
	x52i_date.day = day;
	x52i_date.month = month;
	x52i_date.year = year;
}


/* commit functions */

static inline void _x52i_commit_textline(uint8_t line)
{
	int8_t settext = x52i_ctrl_settext;
	_set(settext, (1 << line));

	for (uint8_t charpair = 0; charpair < 8; ++charpair) {
		x52d_control(x52d, x52i_text.lines[line][charpair], settext);
	}
}

static inline void _x52i_commit_text(void)
{
	for (uint8_t line = x52i_line_0; line <= x52i_line_2; ++line) {
		if (_get(x52i_text.dirty, (1 << line))) {
			int8_t cleartext = x52i_ctrl_cleartext;
			_set(cleartext, (1 << line));
			x52d_control(x52d, 0, cleartext);
			_x52i_commit_textline(line);
			_clr(x52i_text.dirty, (1 << line));
		}
	}
}

static inline void _x52i_commit_led(void)
{
	for (uint8_t led = x52i_led_missile; led <= x52i_led_throttle; ++led) {
		if (_get(x52i_led.dirty, (1 << led))) {
			uint16_t l = _get(x52i_led.state, (1 << led))
					| ((led+1) << 8);
			x52d_control(x52d, l, x52i_ctrl_setled);
			_clr(x52i_led.dirty, (1 << led));
		}
	}
}

static inline void _x52i_commit_time(void)
{
	uint16_t time;
	if (!x52i_time.dirty)
		return;
	time = x52i_time.minute | (x52i_time.hour << 8);
	if (x52i_time.mode24)
		time |= x52i_ctrl_24hourmode;
	x52d_control(x52d, time, x52i_ctrl_time);
	x52i_time.dirty = 0;
}

static inline void _x52i_commit_zone(void)
{
	for (uint8_t off = x52i_offset_0; off <= x52i_offset_1; ++off) {
		if (_get(x52i_zone.dirty, (1 << off))) {
			int8_t time = x52i_ctrl_time;
			_set(time, (1 << off));
			int16_t o = x52i_zone.offset[off];
			if (o < 0)
				o = (-o)|x52i_ctrl_negativeoffset;
			if (x52i_zone.mode24[off])
				o |= x52i_ctrl_24hourmode;
			x52d_control(x52d, o, time);
			_clr(x52i_zone.dirty, (1 << off));
		}
	}
}

static inline void _x52i_commit_date(void)
{
	if (!x52i_date.dirty)
		return;
	x52d_control(x52d, (x52i_date.day | (x52i_date.month << 8)),
		x52i_ctrl_daymonth);
	x52d_control(x52d, x52i_date.year, x52i_ctrl_year);
	x52i_date.dirty = 0;
}

static inline void _x52i_commit_brightness(void)
{
	for (uint8_t d = x52i_brightness_mfd; d <= x52i_brightness_led; ++d) {
		if (_get(x52i_bright.dirty, (1 << d))) {
			int8_t bright = x52i_ctrl_brightness;
			_set(bright, (1 << d));
			x52d_control(x52d, x52i_bright.value[d], bright);
			_clr(x52i_bright.dirty, (1 << d));
		}
	}
}

void x52i_commit(void)
{
	if (!x52d)
		return;
	_x52i_commit_text();
	_x52i_commit_led();
	_x52i_commit_time();
	_x52i_commit_zone();
	_x52i_commit_date();
	_x52i_commit_brightness();
}

void x52i_reset_state(void)
{
	memset(&x52i_time, 0, sizeof(struct x52i_time_t));
	memset(&x52i_zone, 0, sizeof(struct x52i_zone_t));
	memset(&x52i_date, 0, sizeof(struct x52i_date_t));
	memset(&x52i_text, 0, sizeof(struct x52i_text_t));
	memset(&x52i_led, 0, sizeof(struct x52i_led_t));
	memset(&x52i_bright, 0, sizeof(struct x52i_bright_t));
}

int32_t x52i_reset_device(uint8_t shutdown)
{
/*
* TODO: sniff usb traffic under windows which control message might be able
* to reset the device
*/
	if (x52d) {
		x52d_close(x52d);
		x52d = 0;
	}
	if(shutdown)
		return 0;

	x52d = x52d_enumerate();
	if (!x52d_ndevices(x52d)) {
		x52d_close(x52d);
		x52d = 0;
		return -1;
	}
	x52i_reset_state();
	return 0;
}

