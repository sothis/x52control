#ifndef _X52INTERFACE_H
#define _X52INTERFACE_H

#include <stdint.h>
#include <stddef.h>

struct x52d_t;

enum x52i_cled
{
	x52i_cled_A		= 0x01,
	x52i_cled_B		= 0x03,
	x52i_cled_D		= 0x05,
	x52i_cled_E		= 0x07,
	x52i_cled_T1_T2		= 0x09,
	x52i_cled_T3_T4		= 0x0b,
	x52i_cled_T5_T6		= 0x0d,
	x52i_cled_coolie	= 0x0f,
	x52i_cled_i		= 0x11,
};

enum x52i_color
{
	x52i_color_red		= 0x00,
	x52i_color_green	= 0x01,
	x52i_color_amber	= 0x03,
};

enum x52i_led
{
	x52i_led_missile	= 0,
	x52i_led_A_red		= 1,
	x52i_led_A_green	= 2,
	x52i_led_B_red		= 3,
	x52i_led_B_green	= 4,
	x52i_led_D_red		= 5,
	x52i_led_D_green	= 6,
	x52i_led_E_red		= 7,
	x52i_led_E_green	= 8,
	x52i_led_T1_T2_red	= 9,
	x52i_led_T1_T2_green	= 10,
	x52i_led_T3_T4_red	= 11,
	x52i_led_T3_T4_green	= 12,
	x52i_led_T5_T6_red	= 13,
	x52i_led_T5_T6_green	= 14,
	x52i_led_coolie_red	= 15,
	x52i_led_coolie_green	= 16,
	x52i_led_i_red		= 17,
	x52i_led_i_green	= 18,
	x52i_led_throttle	= 19,
};

enum x52i_status
{
	x52i_status_off		= 0,
	x52i_status_on		= 1,
};

enum x52i_line
{
	x52i_line_0		= 0,
	x52i_line_1		= 1,
	x52i_line_2		= 2,
};

enum x52i_offset
{
	x52i_offset_0		= 0,
	x52i_offset_1		= 1,
};

enum x52i_mode
{
	x52i_mode_12h		= 0,
	x52i_mode_24h		= 1,
};

enum x52i_brightness
{
	x52i_brightness_mfd	= 0,
	x52i_brightness_led	= 1,
};


/* callbacks */ /*TODO: put these into x52session */

extern int32_t
x52i_enable();

extern void
x52i_disable();


/* state modifiers */

/**
** sets the brightness of all led's (including those which can't be turned off
** with x52i_set_leds(), like the multicolor-led indicating the current joystick
** mode and the green one in the center coolie hat) or the multifunction
** display. value can anything between 0 (dark) and 127 (bright)
**/
extern void
x52i_set_brightness(enum x52i_brightness dev, uint8_t val);

/**
** turn a specific led on or off.
**/
extern void
x52i_set_leds(enum x52i_led led, enum x52i_status status);

/**
** set all led's on or off.
**/
extern void
x52i_set_leds_all(enum x52i_status status);

/**
** toogle a specific led-group, determined by their color, on or off.
** note that the throttle led and the missilelaunch button led don't have
** specific colors. so they won't be affected by these functions.
** use x52i_set_leds() with x52i_led_missile and x52i_led_throttle to modify
** them.
**/
extern void
x52i_set_leds_color(enum x52i_cled l, enum x52i_color c,
enum x52i_status status);

/**
** toogle all led-groups, determined by their color, on or off.
**/
extern void
x52i_set_leds_color_all(enum x52i_color c, enum x52i_status status);

/**
** sets the text of a specific line on the multifunction display.
** to clear a line set txt to NULL or let it point to an empty string.
** txt can point to a string longer than 16 characters, but only the first
** 16 characters will be copied to the device.
**/
extern void
x52i_set_text(enum x52i_line line, const char* txt);

/**
** sets the time of the clock referenced as clock 1 by the X52 device.
** hour can be anything between 0 and 23, minute between 0 and 59, regardless if
** in 12h or 24h mode. the joystick device does necessary transforms and
** displays 'am' and 'pm' suffixes accordingly.
**/
extern void
x52i_set_time(uint8_t hour, uint8_t minute, enum x52i_mode mode);

/**
** sets the time of the clocks referenced as clock 2 and 3 by the X52 device.
** these clocks are offsetbased taking clock 1 as reference.
** offset can anything between -1023 and +1023, regardless if in 12h or 24h mode.
** it represents the offset in minutes based on the time set with
** x52i_set_time(). the meaningful limits are -720 and +720, which is -12h and
** +12h. the offset driven clocks can be independently setup to be either in 12h
** or 24h mode.
**/
extern void
x52i_set_zone(enum x52i_offset offset, int16_t value, enum x52i_mode mode);


/**
** sets the date.
** year and month can anything between 0 and 15 (well done Saitek! you're all
** capitalistic morons >.<), day 0-31.
**/
extern void
x52i_set_date(uint8_t day, uint8_t month, uint8_t year);


/* committer */

/**
** commit changes to the device at once.
**/
extern void
x52i_commit(void);

/**
** resets the whole internal state.
**/

extern void
x52i_reset(void);

#endif /* _X52INTERFACE_H */

