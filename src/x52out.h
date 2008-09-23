#ifndef X52OUT_H
#define X52OUT_H

#include <pthread.h>
#include "x52tools.h"

struct usb_dev_handle;

class x52out_t : public x52tools_t, public x52listener_t
{
	public:
		x52out_t(void);
		~x52out_t(void);
		void refresh(void* param);
		void print(const char* text, ...);
		void display_brightness(char brightness);
		void led_brightness(char brightness);
		void clear(void);
		void set_verbose(bool verb);
		const out_param_t& current_state(void);
		pthread_mutex_t& parameter_mutex(void);

		// flight loop callback, don't call this one. use refresh() to update the joystick state
		static float update(float elapsed_lastcall, float elapsed_lastloop, int n_loop, void* arg);
	private:
		void setbrightness(bool mfd, char brightness);
		void settext(int line, const char *text, int length);
		void cleartext(int line);
		usb_dev_handle*	a_usbhdl;
		pthread_mutex_t param_mutex;
		out_param_t current_param;
		int product;
};

#endif /* X52OUT_H */

