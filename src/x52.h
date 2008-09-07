#ifndef _X52_H
#define _X52_H

#ifdef LIN
#include <linux/joystick.h>
#include <pthread.h>
#endif

#ifdef APL
#include <stdio.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sysexits.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/usb/USB.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#endif

#include <string>
#include <map>

#include "XPLM/XPLMDataAccess.h"
#include "usbifce.h"

// Buttons

#define X52B_PRIMARY_FIRE       0x00
#define X52B_SECONDARY_FIRE     0x01
#define X52B_BUTTON_A           0x02
#define X52B_BUTTON_B           0x03
#define X52B_BUTTON_C           0x04
#define X52B_PINKY              0x05
#define X52B_BUTTON_D           0x06
#define X52B_BUTTON_E           0x07
#define X52B_BUTTON_T1          0x08
#define X52B_BUTTON_T2          0x09
#define X52B_BUTTON_T3          0x0A
#define X52B_BUTTON_T4          0x0B
#define X52B_BUTTON_T5          0x0C
#define X52B_BUTTON_T6          0x0D
#define X52B_PRIMARY_FIRE_EXT   0x0E
#define X52B_LEFT_MOUSE         0x0F
#define X52B_SCROLLWHEEL_UP     0x10
#define X52B_SCROLLWHEEL_DOWN   0x11
#define X52B_SCROLLWHEEL_PRESS  0x12
#define X52B_BLACKHAT_UP        0x13
#define X52B_BLACKHAT_RIGHT     0x14
#define X52B_BLACKHAT_DOWN      0x15
#define X52B_BLACKHAT_LEFT      0x16
#define X52B_THROTTLEHAT_DOWN   0x17
#define X52B_THROTTLEHAT_RIGHT  0x18
#define X52B_THROTTLEHAT_UP     0x19
#define X52B_THROTTLEHAT_LEFT   0x1A
#define X52B_MODE_1_RED         0x1B
#define X52B_MODE_2_MAGENTA     0x1C
#define X52B_MODE_3_BLUE        0x1D
#define X52B_BUTTON_I           0x1E
#define X52B_FUNCTION_PRESS     0x1F
#define X52B_START_STOP         0x20
#define X52B_RESET              0x21
#define X52B_FUNCTION_UP        0x22
#define X52B_FUNCTION_DOWN      0x23
#define X52B_SELECT_UP          0x24
#define X52B_SELECT_DOWN        0x25
#define X52B_SELECT_PRESS       0x26

// Axes

#define X52A_STICK_X            0x00
#define X52A_STICK_Y            0x01
#define X52A_THROTTLE           0x02
#define X52A_ROTARY_I           0x03
#define X52A_ROTRY_E            0x04
#define X52A_STICK_Z_TWIST      0x05
#define X52A_SMOOTHSLIDER       0x06
#define X52A_SILVERHAT_X        0x07
#define X52A_SILVERHAT_Y        0x08
#define X52A_MOUSE_X            0x09
#define X52A_MOUSE_Y            0x0A

// LED's

#define X52L_SECONDARY_FIRE     0x01
#define X52L_BUTTON_A_RED       0x02
#define X52L_BUTTON_A_GREEN     0x03
#define X52L_BUTTON_B_RED       0x04
#define X52L_BUTTON_B_GREEN     0x05
#define X52L_BUTTON_D_RED       0x06
#define X52L_BUTTON_D_GREEN     0x07
#define X52L_BUTTON_E_RED       0x08
#define X52L_BUTTON_E_GREEN     0x09
#define X52L_BUTTON_T1_RED      0x0A
#define X52L_BUTTON_T1_GREEN    0x0B
#define X52L_BUTTON_T2_RED      0x0C
#define X52L_BUTTON_T2_GREEN    0x0D
#define X52L_BUTTON_T3_RED      0x0E
#define X52L_BUTTON_T3_GREEN    0x0F
#define X52L_BLACKHAT_RED       0x10
#define X52L_BLACKHAT_GREEN     0x11
#define X52L_BUTTON_I_RED       0x12
#define X52L_BUTTON_I_GREEN     0x13
#define X52L_THROTTLESCALE      0x14

// MFD pages

#define MFD_RADIO_COM           0x00
#define MFD_RADIO_NAV           0x01
#define MFD_RADIO_ADF           0x02

// X-Plane DataRefs

#define RADIO_COM1              "sim/cockpit/radios/com1_freq_hz"
#define RADIO_COM2              "sim/cockpit/radios/com2_freq_hz"
#define RADIO_NAV1              "sim/cockpit/radios/nav1_freq_hz"
#define RADIO_NAV2              "sim/cockpit/radios/nav2_freq_hz"
#define RADIO_ADF1              "sim/cockpit/radios/adf1_freq_hz"
#define RADIO_ADF2              "sim/cockpit/radios/adf2_freq_hz"
#define RADIO_COM1_STDBY        "sim/cockpit/radios/com1_stdby_freq_hz"
#define RADIO_COM2_STDBY        "sim/cockpit/radios/com2_stdby_freq_hz"
#define RADIO_NAV1_STDBY        "sim/cockpit/radios/nav1_stdby_freq_hz"
#define RADIO_NAV2_STDBY        "sim/cockpit/radios/nav2_stdby_freq_hz"
#define RADIO_ADF1_STDBY        "sim/cockpit/radios/adf1_stdby_freq_hz"
#define RADIO_ADF2_STDBY        "sim/cockpit/radios/adf2_stdby_freq_hz"

#define INSTRUMENT_BRIGHTNESS   "sim/cockpit/electrical/instrument_brightness"

class X52;

#if APL
/*

*/
typedef struct _sx52device
{
	IOHIDDeviceInterface ** interface;
    
	char product[256];
	long usage;
	long usagePage;
    
	long axes;
	long buttons;
	long hats;
	long elements;    
    
	int removed;
	int uncentered;
} x52device;
//typedef struct _sx52device x52device;

typedef struct _element
{
	IOHIDElementCookie cookie;
} element;
//typedef struct _element element;

typedef struct _elemhandlerarg
{
    X52* sender;
    x52device* pDevice;
} elemhandlerarg;
#endif

class X52
{
public:
    X52(void);
    ~X52(void);

    bool isConnected(void) const;
    void start(unsigned short ms = 100);
    void stop(bool join = true);
    void pause(bool pause);
    void set_interval(unsigned short ms = 100);

private:
    static void* x52_thread(void* arg);
    void x52_update(void);
    void displaydata(int& ticks);
    enum X52PRODUCTS
    {
        OTHER,
        X52STD,
        X52PRO,
        YOKE
    };
    
    static void* input_thread(void* arg);
    void process_input(void);
#if LIN
    void dispatch_input(struct js_event& e);
#endif

#if APL
    void dispatch_input(int number, int value);
    void HIDGetElementInfo (CFTypeRef refElement, element* pElement);
    void HIDAddElement (CFTypeRef refElement, x52device* pDevice);
    static void HIDGetElementsCFArrayHandler (const void* value, void* parameter);
    void HIDGetElements (CFTypeRef refElementCurrent, x52device* pDevice);
    void HIDGetCollectionElements (CFMutableDictionaryRef deviceProperties, x52device* pDevice);
    CFMutableDictionaryRef setup_dictionary(UInt32 usagePage, UInt32 usage);
    io_iterator_t find_device(const mach_port_t masterPort, UInt32 usagePage, UInt32 usage);
    int is_x52product(io_registry_entry_t hidDevice);
    IOHIDDeviceInterface** create_deviceinterface(io_object_t hidDevice);
    void event_interface(IOHIDDeviceInterface **hidDeviceInterface);
    void process_device(io_object_t hidDevice);
    int mac_jsstart();
#endif
     void datacycle_up(void);
    void datacycle_down(void);

    void set_currentpage(int page);
    void add_datasource(std::string& source);
    void add_mfdpage(int type, std::string& name);

    void initX52(void);
    void setText(unsigned char line, std::string &text);
    void clearText(unsigned char line);
    void clearAllText(void);
    void setLed(int led, bool state = true);
    void setLedBrightness(unsigned char value = 0x7F);
    void setMfdBrightness(unsigned char value = 0x7F);
    void setTime(unsigned char hour = 0, unsigned char minute = 0);
    void setDate(unsigned char day = 0, unsigned char month = 0, unsigned short year = 0);
    void setTimezone(unsigned char index, signed short offsetMinutes);

    x52* _x52device;
    int _devfd;
    unsigned int _isRunning;
    unsigned short _x52_interval;
    bool _pause;

#if APL
    x52device curDevice;
    element curElement;
    IOHIDElementCookie buttonElements[256];
    int buttonCookies[256];
    elemhandlerarg ehArgs;
#endif


    pthread_t _ptX52;
    pthread_t _ptInput;
    pthread_mutex_t* _mfd_mutex;

    std::map<std::string, std::pair<XPLMDataRef, int> > datasources;
    std::map<std::string, std::pair<XPLMDataRef, int> >::iterator ds_iter;
    std::map<int, std::string> mfd_pages;
    std::map<int, std::string>::iterator mp_iter;
    std::string currentsource;
    XPLMDataRef currentref;
    int currenttype;
    std::string currentdata;
    XPLMDataRef time_local_ref;
    XPLMDataRef time_zulu_ref;
    float current_local_time;
    float current_zulu_time;
    int currentpage;
    std::string current_pagename;
    int curr_int;
    float curr_float;
    double curr_double;
    int com1;
    int com2;
    int nav1;
    int nav2;
    int adf1;
    int adf2;
    int com1s;
    int com2s;
    int nav1s;
    int nav2s;
    int adf1s;
    int adf2s;
    float instr_bri;

    bool datachanged;
    bool useXPlaneTime;
    bool _24hoursLocalTime;
    bool _24hoursZone1;
    bool _24hoursZone2;
};

#endif /* _X52_H */
