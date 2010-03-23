#include "x52interface.h"
#include "x52device.h"

#include <string.h>


#define _set(x, y)	(x |= y)
#define _clr(x, y)	(x &= ~y)
#define _tst(x, y)	((x & y) == y)
#define _setall(x)	(x |= ~0)
#define _clrall(x)	(x ^= x)
#define _bit(x)		(1 << x)

/**
** the usb control message indices.
** we calculate further values from a specific base where possible.
**/
enum x52i_control
{
	x52i_ctrl_negative		= 0x0400,
	x52i_ctrl_brightness		= 0xb0,
	x52i_ctrl_blink_clutch		= 0xb4,
	x52i_ctrl_setled		= 0xb8,
	x52i_ctrl_time			= 0xc0,
	x52i_ctrl_date			= 0xc4,
	x52i_ctrl_text			= 0xd0,
	x52i_ctrl_clearflag		= 0x08,
	x52i_ctrl_shiftindicator	= 0xfd,

	x52i_ctrl_led_min		= 1,
	x52i_ctrl_led_max		= 20,
	x52i_bright_base		= x52i_bright_mfd,
	x52i_zone_base			= x52i_time_zone1,
	x52i_text_base			= x52i_text_line1,
};


/**
** holds the complete device state. is used by x52i_commit() and state
** modifier functions.
**/
static struct x52i_state_t {
	uint32_t	dirty;
	uint32_t	ledstates;
	uint16_t	dat[29];
	uint16_t	lines[3][8];
} x52i_state = {0};


static struct x52d_t*		x52d = 0;


/* shift indicator modifier/committer */

void
x52i_set_shift(enum x52i_status status)
{
	if (x52i_state.dat[x52i_misc_shift] == status)
		return;
	x52i_state.dat[x52i_misc_shift] = status;
	_set(x52i_state.dirty, _bit(x52i_misc_shift));
}

static inline void
_x52i_commit_shift(void)
{
	if (_tst(x52i_state.dirty, _bit(x52i_misc_shift))) {
			x52d_control(x52d,
				x52i_state.dat[x52i_misc_shift],
				x52i_ctrl_shiftindicator);
	}
}


/* clutchbutton led blink modifier/committer */

void
x52i_set_clutchblink(enum x52i_status status)
{
	if (x52i_state.dat[x52i_misc_clutchblink] == status)
		return;
	x52i_state.dat[x52i_misc_clutchblink] = status;
	_set(x52i_state.dirty, _bit(x52i_misc_clutchblink));
}

static inline void
_x52i_commit_clutchblink(void)
{
	if (_tst(x52i_state.dirty, _bit(x52i_misc_clutchblink))) {
			x52d_control(x52d,
				x52i_state.dat[x52i_misc_clutchblink],
				x52i_ctrl_blink_clutch);
	}
}


/* brightness modifier/committer */

void
x52i_set_brightness(enum x52i_device dev, uint8_t val)
{
	if (x52i_state.dat[dev] == val)
		return;
	x52i_state.dat[dev] = val;
	_set(x52i_state.dirty, _bit(dev));
}

static inline void
_x52i_commit_brightness(void)
{
	for (uint8_t i = x52i_bright_mfd; i <= x52i_bright_led; i++) {
		if (_tst(x52i_state.dirty, _bit(i))) {
			register uint8_t d = i - x52i_bright_base;
			register uint8_t c = x52i_ctrl_brightness;
			_set(c, _bit(d));
			x52d_control(x52d, x52i_state.dat[i], c);
		}
	}
}


/* led modifier/committer */

static inline void
_x52i_update_led_data(uint32_t leds)
{
	for (uint8_t i = x52i_ctrl_led_min; i <= x52i_ctrl_led_max; i++) {
		register uint16_t v = _tst(x52i_state.ledstates, _bit(i))
					| (i << 8);
		if (x52i_state.dat[i] == v)
			continue;
		x52i_state.dat[i] = v;
		_set(x52i_state.dirty, _bit(i));
	}
}

void
x52i_set_led(uint32_t leds)
{
	_set(x52i_state.ledstates, leds);
	_x52i_update_led_data(leds);
}

void
x52i_clr_led(uint32_t leds)
{
	_clr(x52i_state.ledstates, leds);
	_x52i_update_led_data(leds);
}

static inline void
_x52i_commit_led(void)
{
	for (uint8_t i = x52i_ctrl_led_min; i <= x52i_ctrl_led_max; i++) {
		if (_tst(x52i_state.dirty, _bit(i)))
			x52d_control(x52d, x52i_state.dat[i], x52i_ctrl_setled);
	}
}


/* text modifier/committer */

void
x52i_set_text(enum x52i_device line, const char* txt)
{
	uint8_t idx = line - x52i_text_base;
	if (!txt) txt = "";

	if (!strncmp((char*)x52i_state.lines[idx], txt, sizeof(uint16_t)*8))
		return;

	strncpy((char*)x52i_state.lines[idx], txt, sizeof(uint16_t)*8);
	_set(x52i_state.dirty, _bit(line));
	#if defined (__ppc__)
	for (uint8_t charpair = 0; charpair < 8; ++charpair) {
		x52i_state.lines[idx][charpair] =
		((x52i_state.lines[idx][charpair] >> 8) & 0xff) |
		((x52i_state.lines[idx][charpair] & 0xff) << 8);
	}
	#endif /* __ppc__ */
}

static inline void
_x52i_commit_text(void)
{
	for (uint8_t i = x52i_text_line1; i <= x52i_text_line3; i++) {
		if (_tst(x52i_state.dirty, _bit(i))) {
			register uint8_t l = i - x52i_text_base;
			register uint8_t c = x52i_ctrl_text;
			_set(c, _bit(l));
			x52d_control(x52d, 0, c | x52i_ctrl_clearflag);
			for (uint8_t ch = 0; ch < 8; ++ch)
				x52d_control(x52d, x52i_state.lines[l][ch], c);
		}
	}
}


/* time modifier/committer */

void
x52i_set_time(uint8_t hour, uint8_t minute, enum x52i_mode mode)
{
	uint16_t val = minute | (hour << 8) | mode;
	if (x52i_state.dat[x52i_time_clock] == val)
		return;
	x52i_state.dat[x52i_time_clock] = val;
	_set(x52i_state.dirty, _bit(x52i_time_clock));
}

static inline void
_x52i_commit_time(void)
{
	if (_tst(x52i_state.dirty, _bit(x52i_time_clock)))
		x52d_control(x52d, x52i_state.dat[x52i_time_clock],
			x52i_ctrl_time);
}


/* timezone modifier/committer */

void
x52i_set_zone(enum x52i_device zone, int16_t value, enum x52i_mode mode)
{
	if (value < 0)
		value = (-value) | x52i_ctrl_negative;
	value |= mode;
	if (x52i_state.dat[zone] == value)
		return;
	x52i_state.dat[zone] = value;
	_set(x52i_state.dirty, _bit(zone));
}

static inline void
_x52i_commit_zone(void)
{
	for (uint8_t i = x52i_time_zone1; i <= x52i_time_zone2; i++) {
		if (_tst(x52i_state.dirty, _bit(i))) {
			register uint8_t z = i - x52i_zone_base;
			register uint8_t c = x52i_ctrl_time;
			_set(c, _bit(z));
			x52d_control(x52d, x52i_state.dat[i], c);
		}
	}
}


/* date modifier/committer */

void
x52i_set_date(uint8_t d1, uint8_t d2, uint8_t d3)
{
	uint16_t val = d1 | (d2 << 8);
	if ((x52i_state.dat[x52i_date_val12] == val) &&
		(x52i_state.dat[x52i_date_val3] == d3))
		return;
	x52i_state.dat[x52i_date_val12] = val;
	x52i_state.dat[x52i_date_val3] = d3;
	_set(x52i_state.dirty, _bit(x52i_date_val12) | _bit(x52i_date_val3));
}

static inline void
_x52i_commit_date(void)
{
	if (_tst(x52i_state.dirty, _bit(x52i_date_val12))) {
		x52d_control(x52d, x52i_state.dat[x52i_date_val12],
			x52i_ctrl_date);
		x52d_control(x52d, x52i_state.dat[x52i_date_val3],
			x52i_ctrl_date + 4);
	}
}

void
x52i_reset_state(void)
{
	memset(&x52i_state, 0, sizeof(struct x52i_state_t));
}


/* global committer */

void
x52i_commit(void)
{
	if (!x52d || !x52i_state.dirty)
		return;
	_x52i_commit_text();
	_x52i_commit_time();
	_x52i_commit_zone();
	_x52i_commit_date();
	_x52i_commit_shift();
	_x52i_commit_clutchblink();
	_x52i_commit_led();
	_x52i_commit_brightness();
	_clrall(x52i_state.dirty);
}


/* maintenance functions */

void
x52i_set_defaults(void)
{
	/* flush internal state buffer with zeros */
	x52i_reset_state();
	/* force committing of all values */
	_setall(x52i_state.dirty);
	/* set led and mfd brightness to maximum */
	x52i_set_brightness(x52i_bright_mfd, 128);
	x52i_set_brightness(x52i_bright_led, 128);
	/* turn on all green and additional led's */
	x52i_clr_led(x52i_led_all);
	x52i_set_led(x52i_led_all_green);
	x52i_set_led(x52i_led_launch);
	x52i_set_led(x52i_led_throttle);
	/* clear all text */
	x52i_set_text(x52i_text_line1, 0);
	x52i_set_text(x52i_text_line2, 0);
	x52i_set_text(x52i_text_line3, 0);
	/* initialize clock module */
	x52i_set_date(0, 0, 0);
	x52i_set_time(0, 0, x52i_mode_24h);
	x52i_set_zone(x52i_time_zone1, 0, x52i_mode_24h);
	x52i_set_zone(x52i_time_zone2, 0, x52i_mode_24h);
	/* misc */
	x52i_set_clutchblink(x52i_status_off);
	x52i_set_shift(x52i_status_off);
	/* commit changes */
	x52i_commit();
}


/* control functions */

int32_t
x52i_open_device(void)
{
	if (x52d)
		x52d_close(x52d);

	x52d = x52d_enumerate();
	if (!x52d_ndevices(x52d)) {
		x52d_close(x52d);
		x52d = 0;
		return -1;
	}
	x52i_set_defaults();
	return 0;
}

void
x52i_close_device(void)
{
	if (!x52d)
		return;
	x52i_set_defaults();
	x52d_close(x52d);
	x52d = 0;
}

