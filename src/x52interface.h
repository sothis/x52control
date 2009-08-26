#ifndef _X52INTERFACE_H
#define _X52INTERFACE_H

#include <stdint.h>


enum x52i_device {
	x52i_led_launch		= 1 << 1,
	x52i_led_A_red		= 1 << 2,
	x52i_led_A_green	= 1 << 3,
	x52i_led_A_amber	= x52i_led_A_red | x52i_led_A_green,
	x52i_led_B_red		= 1 << 4,
	x52i_led_B_green	= 1 << 5,
	x52i_led_B_amber	= x52i_led_B_red | x52i_led_B_green,
	x52i_led_D_red		= 1 << 6,
	x52i_led_D_green	= 1 << 7,
	x52i_led_D_amber	= x52i_led_D_red | x52i_led_D_green,
	x52i_led_E_red		= 1 << 8,
	x52i_led_E_green	= 1 << 9,
	x52i_led_E_amber	= x52i_led_E_red | x52i_led_E_green,
	x52i_led_T1T2_red	= 1 << 10,
	x52i_led_T1T2_green	= 1 << 11,
	x52i_led_T1T2_amber	= x52i_led_T1T2_red | x52i_led_T1T2_green,
	x52i_led_T3T4_red	= 1 << 12,
	x52i_led_T3T4_green	= 1 << 13,
	x52i_led_T3T4_amber	= x52i_led_T3T4_red | x52i_led_T3T4_green,
	x52i_led_T5T6_red	= 1 << 14,
	x52i_led_T5T6_green	= 1 << 15,
	x52i_led_T5T6_amber	= x52i_led_T5T6_red | x52i_led_T5T6_green,
	x52i_led_pov2_red	= 1 << 16,
	x52i_led_pov2_green	= 1 << 17,
	x52i_led_pov2_amber	= x52i_led_pov2_red | x52i_led_pov2_green,
	x52i_led_clutch_red	= 1 << 18,
	x52i_led_clutch_green	= 1 << 19,
	x52i_led_clutch_amber	= x52i_led_clutch_red | x52i_led_clutch_green,
	x52i_led_throttle	= 1 << 20,
	x52i_led_all_red	= x52i_led_A_red | x52i_led_B_red
				| x52i_led_D_red | x52i_led_E_red
				| x52i_led_T1T2_red | x52i_led_T3T4_red
				| x52i_led_T5T6_red | x52i_led_pov2_red
				| x52i_led_clutch_red,
	x52i_led_all_green	= x52i_led_A_green | x52i_led_B_green
				| x52i_led_D_green | x52i_led_E_green
				| x52i_led_T1T2_green | x52i_led_T3T4_green
				| x52i_led_T5T6_green | x52i_led_pov2_green
				| x52i_led_clutch_green,
	x52i_led_all_amber	= x52i_led_all_green | x52i_led_all_red,
	x52i_led_all		= x52i_led_launch | x52i_led_throttle
				| x52i_led_all_amber,

	x52i_misc_clutchblink	= 0,
	/* 1..20: leds */
	x52i_misc_shift		= 21,
	x52i_bright_mfd		= 22,
	x52i_bright_led		= 23,
	x52i_time_clock		= 24,
	x52i_time_zone1		= 25,
	x52i_time_zone2		= 26,
	x52i_date_val12		= 27,
	x52i_date_val3		= 28,
	x52i_text_line1		= 29,
	x52i_text_line2		= 30,
	x52i_text_line3		= 31,
};

enum x52i_status
{
	x52i_status_off		= 0x50,
	x52i_status_on		= 0x51,
};

enum x52i_mode
{
	x52i_mode_12h		= 0x0000,
	x52i_mode_24h		= 0x8000,
};


/* state modifiers */

/**
** toggle the shift indicator on the multifunction display.
**/
extern void
x52i_set_shift(enum x52i_status status);


/**
** toggle blinking of the clutchbutton (i-button) and center-cooliehat led.
**/
extern void
x52i_set_clutchblink(enum x52i_status status);


/**
** sets the brightness of all led's (including those which can't be turned off
** with x52i_clr_led(), like the multicolor-led indicating the current joystick
** mode and the green one in the center coolie hat) and the multifunction
** display. value can anything between 0 (dark) and 128 (bright)
**/
extern void
x52i_set_brightness(enum x52i_device dev, uint8_t val);


/**
** turn specified leds on.
**/
extern void
x52i_set_led(uint32_t leds);


/**
** turn specified leds off.
**/
extern void
x52i_clr_led(uint32_t leds);


/**
** sets the text of a specific line on the multifunction display.
** to clear a line set txt to NULL or let it point to an empty string.
** txt can point to a string longer than 16 characters, but only the first
** 16 characters will be copied to the device.
**/
extern void
x52i_set_text(enum x52i_device line, const char* txt);


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
** offset can anything between -1023 and +1023, regardless if in 12h or 24h
** mode. it represents the offset in minutes based on the time set with
** x52i_set_time(), so in hours the limits are -17 and +17 respectively.
** the offset driven clocks can be independently setup to be either in 12h
** or 24h mode.
**/
extern void
x52i_set_zone(enum x52i_device zone, int16_t value, enum x52i_mode mode);


/**
** sets the date. the display has 3 digit groups, each can display 2-digit
** numbers. digit group 1 and 2 support numbers from 0 to 39, digit group
** 3 from 0 to 63. so until year 2039 any day-month-year combination is
** possible. must be called once at least to activate the date-time display
** on the mfd.
**/
extern void
x52i_set_date(uint8_t d1, uint8_t d2, uint8_t d3);


/**
** resets the internal state buffer.
**/
extern void
x52i_reset_state(void);


/* committer, device control and maintenance */

/**
** commit changes of the internal state buffer to the device at once.
**/
extern void
x52i_commit(void);

/**
** fills the internal state buffer with default values and commits them
** to the device immediately.
**/
extern void
x52i_set_defaults(void);

/**
** tries to open a connection to a X52 device. if successful it initializes
** the device by calling x52i_set_defaults(). if there was already a connection,
** x52i_open_device() closes it and opens a new one. returns 0 on success, -1
** when no X52 device was found.
**/
extern int32_t
x52i_open_device(void);

/**
** resets the device by calling x52i_set_defaults(). after that it closes the
** device and frees all allocated resources.
**/
extern void
x52i_close_device(void);

#endif /* _X52INTERFACE_H */

