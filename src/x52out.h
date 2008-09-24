#ifndef X52OUT_H
#define X52OUT_H

#include <string>
#include "x52tools.h"

struct usb_dev_handle;

class x52out_t : public x52tools_t
{
	public:
		x52out_t(void);
		~x52out_t(void);

		void set_textdata(const char* text);
		void print(const char* text, ...);
		void print();
		void display_brightness(char brightness);
		void led_brightness(char brightness);
		void clear(void);
	
	private:
		void setbrightness(bool mfd, char brightness);
		void settext(int line, const char *text, int length);
		void cleartext(int line);
		usb_dev_handle*	a_usbhdl;
		int product;
		std::string a_textdata;

};

#endif /* X52OUT_H */

