#include "x52session.h"
#include "x52interface.h"

static inline void _x52s_init(void)
{
	/* set led and mfd brightness to maximum */
	x52i_set_brightness(x52i_brightness_mfd, 127);
	x52i_set_brightness(x52i_brightness_led, 127);
	/* turn on all green and additional led's */
	x52i_set_led_all(x52i_status_off);
	x52i_set_led(x52i_led_missile, x52i_status_on);
	x52i_set_led(x52i_led_throttle, x52i_status_on);
	x52i_set_led_color_all(x52i_color_green, x52i_status_on);
	/* clear all text */
	x52i_set_text(x52i_line_0, 0);
	x52i_set_text(x52i_line_1, 0);
	x52i_set_text(x52i_line_2, 0);
	/* initialize clock module */
	x52i_set_date(0, 0, 0);
	x52i_set_time(0, 0, x52i_mode_24h);
	x52i_set_zone(x52i_offset_0, 0, x52i_mode_24h);
	x52i_set_zone(x52i_offset_1, 0, x52i_mode_24h);
	/* commit changes */
	x52i_commit();
}


/* callbacks */

int32_t
x52s_enable()
{
	/* try to open device, reset internal state */
	int32_t res = x52i_reset_device(0);
	if (res != 0)
		return 0;
	_x52s_init();
	return 1;
}

void
x52s_disable()
{
	x52i_reset_device(0);
	_x52s_init();
	/* close device and free allocated resources */
	x52i_reset_device(1);
}

