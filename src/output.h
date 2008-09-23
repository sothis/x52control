#ifndef OUTPUT_H
#define OUTPUT_H

#include <pthread.h>

struct usb_dev_handle;

typedef struct out_param_t
{
	float	next_call;
	bool	update;
	char	text[51];
} out_param_t;

class x52out_t
{
	public:
		x52out_t(void);
		~x52out_t(void);
		void refresh(const out_param_t* param);
		void print(const char* text);
		void clear(void);
		void debug_out(int type, const char* msg, ...);
		void set_verbose(bool verb);
		const out_param_t& current_state(void);
		pthread_mutex_t& parameter_mutex(void);

		// flight loop callback, call refresh to update joystick state
		static float update(float elapsed_lastcall, float elapsed_lastloop, int n_loop, void* arg);
	private:
		void settext(int line, const char *text, int length);
		void cleartext(int line);
		usb_dev_handle*	a_usbhdl;
		pthread_mutex_t param_mutex;
		out_param_t current_param;
		bool verbose;
		int product;
};

#endif /* OUTPUT_H */

