#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <errno.h>
#include <string.h>

#include <sstream>
#include "x52.h"

/* admittedly, this code is a mess */

X52::X52(void) : _isRunning(0), _x52_interval(250), _pause(0), currentref(NULL), currenttype(0)
{
    _x52device = x52_init();
    if (!_x52device) return;
    _mfd_mutex = NULL;
    initX52();
    _24hoursLocalTime = true;
    _24hoursZone1 = true;
    _24hoursZone2 = true;
    useXPlaneTime = true;
    datachanged = true;
    currentpage = 0;
    _devfd = -1;
    _devfd = open("/dev/js0", O_RDONLY);
#if APL
    curDevice.buttons = 0;
    curDevice.axes = 0;
    curDevice.hats = 0;
#endif
    return;
}

X52::~X52(void)
{
    if (isConnected())
    {
        clearAllText();
        setTime();
        setTimezone(0, 0);
        setTimezone(1, 0);
        setDate();
        setMfdBrightness();
        setLedBrightness();
        stop();
        x52_close(_x52device);
    }
    return;
}

bool X52::isConnected(void) const
{
    if (!_x52device) return false;
    return true;
}

void X52::set_currentpage(int page)
{
    if (_mfd_mutex) pthread_mutex_lock(_mfd_mutex);
    mp_iter = mfd_pages.find(page);
    if (mp_iter == mfd_pages.end())
    {
        if (_mfd_mutex) pthread_mutex_unlock(_mfd_mutex);
        return;
    }
    currentpage = mp_iter->first;
    current_pagename = mp_iter->second;
    if (_mfd_mutex) pthread_mutex_unlock(_mfd_mutex);
    return;
}

void X52::datacycle_up(void)
{
    if (_mfd_mutex) pthread_mutex_lock(_mfd_mutex);
    mp_iter = mfd_pages.find(currentpage);
    mp_iter++;
    if (mp_iter == mfd_pages.end()) mp_iter = mfd_pages.begin();
    currentpage = mp_iter->first;
    current_pagename = mp_iter->second;
    datachanged = true;
    if (_mfd_mutex) pthread_mutex_unlock(_mfd_mutex);
    return;
}

void X52::datacycle_down(void)
{
    if (_mfd_mutex) pthread_mutex_lock(_mfd_mutex);
    mp_iter = mfd_pages.find(currentpage);
    if (mp_iter == mfd_pages.begin()) mp_iter = mfd_pages.end();
    mp_iter--;
    currentpage = mp_iter->first;
    current_pagename = mp_iter->second;
    datachanged = true;
    if (_mfd_mutex) pthread_mutex_unlock(_mfd_mutex);
    return;
}

void X52::add_datasource(std::string& source)
{
    XPLMDataRef temp = NULL;
    int reftype = 0;

    temp = XPLMFindDataRef(source.c_str());
    if (!temp)
    {
        fprintf(stderr, "data source not found.\n");
        return;
    }
    reftype = XPLMGetDataRefTypes(temp);
    if (!reftype)
    {
        fprintf(stderr, "data type not applicable\n");
        return;
    }
    if (_mfd_mutex) pthread_mutex_lock(_mfd_mutex);
    datasources[source] = std::make_pair(temp, reftype);
    if (_mfd_mutex) pthread_mutex_unlock(_mfd_mutex);
    fprintf(stderr, "added data source: %s\n", source.c_str());
    return;
}

void X52::add_mfdpage(int type, std::string& name)
{
    if (_mfd_mutex) pthread_mutex_lock(_mfd_mutex);
    mfd_pages[type] = name;
    if (_mfd_mutex) pthread_mutex_unlock(_mfd_mutex);
}

void X52::displaydata(int& ticks)
{
    std::string t1, t2, t3;
    char buf[20] = {};

    if (time_local_ref && time_zulu_ref && !ticks)
    {
        unsigned char hours;
        unsigned char mins;
        unsigned int  secs;
        unsigned int  zsecs;
        signed int diff;
        time_t t;
        struct tm tr;

        t = time(NULL);
        localtime_r(&t, &tr);
        if (useXPlaneTime)
        {
            current_local_time = XPLMGetDataf(time_local_ref);
            current_zulu_time = XPLMGetDataf(time_zulu_ref);
            secs = current_local_time;
            zsecs = current_zulu_time;
            diff = zsecs - secs;
            hours = secs/3600;
            mins = (secs%3600)/60;
            secs = (secs%3600)%60;
            setTime(hours, mins);
            //  setTimezone(0, diff/60);
        }
        else
        {
            setTime(tr.tm_hour, tr.tm_min);
        }
        setDate(tr.tm_mday,tr.tm_mon+1,1900+tr.tm_year);
    }
    if (_mfd_mutex) pthread_mutex_lock(_mfd_mutex);
    for (ds_iter = datasources.begin(); ds_iter != datasources.end(); ++ds_iter)
    {
        currentref =  (ds_iter->second).first;
        currenttype = (ds_iter->second).second;
        currentsource = ds_iter->first;
        switch (currenttype)
        {
        case xplmType_Int:
            curr_int = XPLMGetDatai(currentref);
            break;
        case xplmType_Float:
            curr_float = XPLMGetDataf(currentref);
            break;
        case xplmType_Double:
            curr_double = XPLMGetDatad(currentref);
            break;
        case xplmType_FloatArray:
            break;
        case xplmType_IntArray:
            break;
        case xplmType_Data:
            break;
        case xplmType_Unknown:
            break;
        default:
            break;
        }
        if ((currentsource == RADIO_ADF1) && (adf1 != curr_int))
        {
            adf1 = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_ADF2) && (adf2 != curr_int))
        {
            adf2 = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_ADF1_STDBY) && (adf1s != curr_int))
        {
            adf1s = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_ADF2_STDBY) && (adf2s != curr_int))
        {
            adf2s = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_COM1) && (com1 != curr_int))
        {
            com1 = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_COM2) && (com2 != curr_int))
        {
            com2 = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_COM1_STDBY) && (com1s != curr_int))
        {
            com1s = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_COM2_STDBY) && (com2s != curr_int))
        {
            com2s = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_NAV1) && (nav1 != curr_int))
        {
            nav1 = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_NAV2) && (nav2 != curr_int))
        {
            nav2 = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_NAV1_STDBY) && (nav1s != curr_int))
        {
            nav1s = curr_int;
            datachanged = true;
        }
        if ((currentsource == RADIO_NAV2_STDBY) && (nav2s != curr_int))
        {
            nav2s = curr_int;
            datachanged = true;
        }
        if ((currentsource == INSTRUMENT_BRIGHTNESS) && ((instr_bri-curr_float) > 0.01f || (instr_bri-curr_float) < -0.01))
        {
            instr_bri = curr_float;
            setMfdBrightness((unsigned char)(instr_bri * 0x7f));
            setLedBrightness((unsigned char)(instr_bri * 0x7f));
        }
    }
    if (_mfd_mutex) pthread_mutex_unlock(_mfd_mutex);
    if (datachanged)
    {
        switch (currentpage)
        {
        case MFD_RADIO_ADF:
            t1 = " Adf1      Adf2";
            memset(buf, 0, 20);
            snprintf(buf, 17, "%03d Hz Ac %03d Hz", adf1, adf2);
            t2.assign(buf);
            memset(buf, 0, 20);
            snprintf(buf, 17, "%03d Hz Sb %03d Hz", adf1s, adf2s);
            t3.assign(buf);
            setText(0, t1);
            setText(1, t2);
            setText(2, t3);
            break;
        case MFD_RADIO_COM:
            t1 = " Com1      Com2";
            memset(buf, 0, 20);
            snprintf(buf, 17, "%06.2f Ac %06.2f", com1/100.0f, com2/100.0f);
            t2.assign(buf);
            memset(buf, 0, 20);
            snprintf(buf, 17, "%06.2f Sb %06.2f", com1s/100.0f, com2s/100.0f);
            t3.assign(buf);
            setText(0, t1);
            setText(1, t2);
            setText(2, t3);
            break;
        case MFD_RADIO_NAV:
            t1 = " Nav1      Nav2";
            memset(buf, 0, 20);
            snprintf(buf, 17, "%06.2f Ac %06.2f", nav1/100.0f, nav2/100.0f);
            t2.assign(buf);
            memset(buf, 0, 20);
            snprintf(buf, 17, "%06.2f Sb %06.2f", nav1s/100.0f, nav2s/100.0f);
            t3.assign(buf);
            setText(0, t1);
            setText(1, t2);
            setText(2, t3);
            break;
        default:
            break;
        }
        if (_mfd_mutex) pthread_mutex_lock(_mfd_mutex);
        datachanged = false;
        if (_mfd_mutex) pthread_mutex_unlock(_mfd_mutex);
    }
    return;
}

void X52::start(unsigned short ms)
{
    unsigned short oi = _x52_interval;

    if (!isConnected() || _isRunning) return;

    _mfd_mutex = new pthread_mutex_t;
    if (!_mfd_mutex) return;

    if (pthread_mutex_init(_mfd_mutex, NULL))
    {
        delete _mfd_mutex;
        return;
    }

    _isRunning = 1;
    if (ms) _x52_interval = ms;
    if (pthread_create(&_ptX52, NULL, x52_thread, this))
    {
        _x52_interval = oi;
        _isRunning = 0;
        pthread_mutex_destroy(_mfd_mutex);
        delete _mfd_mutex;
        return;
    }

    if (pthread_create(&_ptInput, NULL, input_thread, this))
    {
        _x52_interval = oi;
        _isRunning = 0;
        pthread_join(_ptX52, NULL);
        pthread_detach(_ptX52);
        pthread_mutex_destroy(_mfd_mutex);
        delete _mfd_mutex;

    }

    return;
}

void X52::stop(bool join)
{
    if (!isConnected() || !_isRunning) return;

    _isRunning = 0;
    close(_devfd);
    if (join)
    {
        // we won't join the input thread on linux, since read
        // blocks, and select is teh suck, maybe trying poll()?
        #if APL
        pthread_join(_ptInput, NULL);
        #endif
    }

    pthread_detach(_ptInput);
    if (join)
    {
        pthread_join(_ptX52, NULL);
    }
    pthread_detach(_ptX52);
    pthread_mutex_destroy(_mfd_mutex);
    delete _mfd_mutex;

    return;
}

void X52::pause(bool pause)
{
    if (!pause)
    {
        _devfd = open("/dev/js0", O_RDONLY);
    }
    else
    {
        close(_devfd);
        _devfd = -1;
    }
    _pause = pause;
    return;
}

void* X52::x52_thread(void* arg)
{

    X52* tmp = reinterpret_cast<X52*>(arg);
    if (tmp) tmp->x52_update();
    pthread_exit(NULL);

	return NULL;
}

void X52::x52_update(void)
{

    int ticks = 0;
    printf("X52 Data thread started\n");
    while (_isRunning)
    {
        usleep(_x52_interval*1000);

        if (ticks == 1000/_x52_interval) ticks = 0;
        displaydata(ticks);
        ticks++;

        while (_pause && _isRunning)
        {
            usleep(50000);
        }
    }

    return;
}

void* X52::input_thread(void* arg)
{
    X52* tmp = reinterpret_cast<X52*>(arg);
    if (tmp) tmp->process_input();
    pthread_exit(NULL);
	return NULL;
}

void X52::process_input(void)
{
    int res = -1;
    printf("X52 Input thread started\n");
#if LIN
    struct js_event jse;

    while (_isRunning)
    {
        if (_devfd != -1)
        {
            res = read(_devfd, &jse, sizeof(struct js_event));
            if (res == -1) perror("error reading joystick event:");
            else if (!(jse.type == 0x80 || jse.type == 0x81 || jse.type == 0x82 || _pause))
            {
                if (jse.type == 0x01) dispatch_input(jse);
            }
        }
        while (_pause && _isRunning)
        {
            usleep(50000);
        }
    }
#endif
#if APL
    mac_jsstart();
#endif
    return;
}
#if LIN
void X52::dispatch_input(struct js_event& e)
{
    switch (e.number)
    {
    case X52B_FUNCTION_UP:
        if (!e.value) break;
        datacycle_up();
        break;
    case X52B_FUNCTION_DOWN:
        if (!e.value) break;
        datacycle_down();
        break;
    default:
        break;
    }

    return;
}
#endif
// Hardware functions

void X52::initX52(void)
{
    std::string ds;
    clearAllText();
    setMfdBrightness(70);
    setLedBrightness(70);
    setTime();
    setDate();
    setTimezone(0, 0);
    setTimezone(1, 0);

    time_local_ref = XPLMFindDataRef("sim/time/local_time_sec");
    time_zulu_ref = XPLMFindDataRef("sim/time/zulu_time_sec");
    if (!time_local_ref)
    {
        printf("timedata source (local) not found.\n");
        return;
    }
    if (!time_zulu_ref)
    {
        printf("timedata source (zulu) not found.\n");
        return;
    }
    ds = RADIO_COM1;
    add_datasource(ds);
    ds = RADIO_COM2;
    add_datasource(ds);
    ds = RADIO_NAV1;
    add_datasource(ds);
    ds = RADIO_NAV2;
    add_datasource(ds);
    ds = RADIO_ADF1;
    add_datasource(ds);
    ds = RADIO_ADF2;
    add_datasource(ds);
    ds = RADIO_COM1_STDBY;
    add_datasource(ds);
    ds = RADIO_COM2_STDBY;
    add_datasource(ds);
    ds = RADIO_NAV1_STDBY;
    add_datasource(ds);
    ds = RADIO_NAV2_STDBY;
    add_datasource(ds);
    ds = RADIO_ADF1_STDBY;
    add_datasource(ds);
    ds = RADIO_ADF2_STDBY;
    add_datasource(ds);
    ds = INSTRUMENT_BRIGHTNESS;
    add_datasource(ds);
    ds = "Radio Com";
    add_mfdpage(MFD_RADIO_COM, ds);
    ds = "Radio Nav";
    add_mfdpage(MFD_RADIO_NAV, ds);
    ds = "Radio Adf";
    add_mfdpage(MFD_RADIO_ADF, ds);
    set_currentpage(MFD_RADIO_COM);
    return;
}

void X52::setText(unsigned char line, std::string &text)
{
    x52_settext(_x52device, line, (char*)text.c_str(), text.size());
    return;
}

void X52::clearText(unsigned char line)
{
    x52_cleartext(_x52device, line);
    return;
}

void X52::clearAllText(void)
{
    x52_clearall(_x52device);
    return;
}

void X52::setLed(int led, bool state)
{
    x52_setled(_x52device, led, state);
    return;
}

void X52::setLedBrightness(unsigned char value)
{
    x52_setbri(_x52device, 0, value);
    return;
}

void X52::setMfdBrightness(unsigned char value)
{
    x52_setbri(_x52device, 1, value);
    return;
}

void X52::setTime(unsigned char hour, unsigned char minute)
{
    x52_settime(_x52device, _24hoursLocalTime, hour, minute);
    return;
}

void X52::setDate(unsigned char day, unsigned char month, unsigned short year)
{
    x52_setdate(_x52device, year, month, day);
    return;
}

void X52::setTimezone(unsigned char index, signed short offsetMinutes)
{
    int realoff = 0;
    int inv = 0;
    if (index > 1) return;
    if (offsetMinutes < 0) inv = 1;
    realoff = abs(offsetMinutes);
    x52_setoffs(_x52device, index, index?_24hoursZone2:_24hoursZone1, inv, realoff);
    return;
}

#if APL

void X52::dispatch_input(int number, int value)
{
    switch (number)
    {
        case X52B_FUNCTION_UP:
            if (!value) break;
            datacycle_up();
            break;
        case X52B_FUNCTION_DOWN:
            if (!value) break;
            datacycle_down();
            break;
        default:
            break;
    }
    return;
}

void X52::HIDGetElementInfo (CFTypeRef refElement, element* pElement)
{
	long number;
	CFTypeRef refType;
    
	refType = CFDictionaryGetValue ((__CFDictionary*)refElement, CFSTR(kIOHIDElementCookieKey));
	if (refType && CFNumberGetValue ((__CFNumber*)refType, kCFNumberLongType, &number))
    {
		pElement->cookie = (IOHIDElementCookie) number;
    }
}			

void X52::HIDAddElement (CFTypeRef refElement, x52device* pDevice)
{
	long elementType, usagePage, usage;
	CFTypeRef refElementType = CFDictionaryGetValue ((__CFDictionary*)refElement, CFSTR(kIOHIDElementTypeKey));
	CFTypeRef refUsagePage = CFDictionaryGetValue ((__CFDictionary*)refElement, CFSTR(kIOHIDElementUsagePageKey));
	CFTypeRef refUsage = CFDictionaryGetValue ((__CFDictionary*)refElement, CFSTR(kIOHIDElementUsageKey));
    
    
	if ((refElementType) && (CFNumberGetValue ((__CFNumber*)refElementType, kCFNumberLongType, &elementType)))
	{
		if ((elementType == kIOHIDElementTypeInput_Misc) || (elementType == kIOHIDElementTypeInput_Button) ||
			(elementType == kIOHIDElementTypeInput_Axis))
		{
			if (refUsagePage && CFNumberGetValue ((__CFNumber*)refUsagePage, kCFNumberLongType, &usagePage) &&
				refUsage && CFNumberGetValue ((__CFNumber*)refUsage, kCFNumberLongType, &usage))
			{
				switch (usagePage)
				{
					case kHIDPage_GenericDesktop:
                    {
                        switch (usage)
                        {
                            case kHIDUsage_GD_X:
                            case kHIDUsage_GD_Y:
                            case kHIDUsage_GD_Z:
                            case kHIDUsage_GD_Rx:
                            case kHIDUsage_GD_Ry:
                            case kHIDUsage_GD_Rz:
                            case kHIDUsage_GD_Slider:
                            case kHIDUsage_GD_Dial:
                            case kHIDUsage_GD_Wheel:
                                HIDGetElementInfo (refElement, &curElement);
                                pDevice->axes++;
								break;
                            case kHIDUsage_GD_Hatswitch:
                                HIDGetElementInfo (refElement, &curElement);
                                pDevice->hats++;
								break;
                        }
						break;
                    }
					case kHIDPage_Button:
                      	HIDGetElementInfo (refElement, &curElement);
                        buttonElements[pDevice->buttons] = curElement.cookie;
                        buttonCookies[(int)curElement.cookie] = pDevice->buttons;
                        pDevice->buttons++;
						break;
					default:
						break;
				}
			}
		}
        else if (kIOHIDElementTypeCollection == elementType)
			HIDGetCollectionElements ((CFMutableDictionaryRef) refElement, pDevice);
	}
}

void X52::HIDGetElementsCFArrayHandler (const void* value, void* parameter)
{
    elemhandlerarg* arg = (elemhandlerarg*) parameter;
	if (CFGetTypeID (value) == CFDictionaryGetTypeID ())
		arg->sender->HIDAddElement ((CFTypeRef) value, arg->pDevice);
}

void X52::HIDGetElements (CFTypeRef refElementCurrent, x52device* pDevice)
{
	CFTypeID type = CFGetTypeID (refElementCurrent);
	if (type == CFArrayGetTypeID()) /* if element is an array */
	{
		CFRange range = {0, CFArrayGetCount ((__CFArray*)refElementCurrent)};
		/* CountElementsCFArrayHandler called for each array member */
        ehArgs.sender = this;
        ehArgs.pDevice = pDevice;
		CFArrayApplyFunction ((__CFArray*)refElementCurrent, range, HIDGetElementsCFArrayHandler, &ehArgs);
	}
}			

void X52::HIDGetCollectionElements (CFMutableDictionaryRef deviceProperties, x52device* pDevice)
{
	CFTypeRef refElementTop = CFDictionaryGetValue (deviceProperties, CFSTR(kIOHIDElementKey));
	if (refElementTop)
		HIDGetElements (refElementTop, pDevice);
}

CFMutableDictionaryRef X52::setup_dictionary(UInt32 usagePage, UInt32 usage)
{
    CFNumberRef refUsage = NULL, refUsagePage = NULL;
    CFMutableDictionaryRef refHIDMatchDictionary = NULL;
    
    // Set up a matching dictionary to search I/O Registry by class name for all HID class devices.
    refHIDMatchDictionary = IOServiceMatching (kIOHIDDeviceKey);
    if (refHIDMatchDictionary != NULL)
    {
        // Add key for device type (joystick, in this case) to refine the matching dictionary.
        refUsagePage = CFNumberCreate (kCFAllocatorDefault, kCFNumberIntType, &usagePage);
        refUsage = CFNumberCreate (kCFAllocatorDefault, kCFNumberIntType, &usage);
        CFDictionarySetValue (refHIDMatchDictionary, CFSTR (kIOHIDPrimaryUsagePageKey), refUsagePage);
        CFDictionarySetValue (refHIDMatchDictionary, CFSTR (kIOHIDPrimaryUsageKey), refUsage);
    }
    else
        printf("Failed to get HID CFMutableDictionaryRef via IOServiceMatching.\n");
    return refHIDMatchDictionary;
}

io_iterator_t X52::find_device(const mach_port_t masterPort, UInt32 usagePage, UInt32 usage)
{
    CFMutableDictionaryRef hidMatchDictionary = NULL;
    IOReturn ioReturnValue = kIOReturnSuccess;
    io_iterator_t hidObjectIterator;
    
    // Set up matching dictionary to search the I/O Registry for HID devices we are interested in. Dictionary reference is NULL if error.
    hidMatchDictionary = setup_dictionary(usagePage, usage);
    if (NULL == hidMatchDictionary)
        printf("Couldn’t create a matching dictionary.\n");
    
    // Now search I/O Registry for matching devices.
    ioReturnValue = IOServiceGetMatchingServices (masterPort, hidMatchDictionary, &hidObjectIterator);
    
    // IOServiceGetMatchingServices consumes a reference to the dictionary, so we don't need to release the dictionary ref.
    hidMatchDictionary = NULL;
    return hidObjectIterator;
}

int X52::is_x52product(io_registry_entry_t hidDevice)
{
    kern_return_t result;
    CFMutableDictionaryRef properties = 0;
    long vendor_id, product_id;
    CFTypeRef dict_value = 0;
    
    result = IORegistryEntryCreateCFProperties(hidDevice, &properties, kCFAllocatorDefault, kNilOptions);
    if ((result != KERN_SUCCESS) || (!properties))
        return 0;
    dict_value = CFDictionaryGetValue(properties, CFSTR(kIOHIDVendorIDKey));
    if (!dict_value)
        return 0;
    if (!CFNumberGetValue((__CFNumber*)dict_value, kCFNumberLongType, &vendor_id))
        return 0;
    if (vendor_id != 0x06A3)
        return 0;
    dict_value = CFDictionaryGetValue(properties, CFSTR(kIOHIDProductIDKey));
    if (!dict_value)
        return 0;
    if (!CFNumberGetValue((__CFNumber*)dict_value, kCFNumberLongType, &product_id))
        return 0;
    switch (product_id)
    {
        case 0x255:
        case 0x75C:
            HIDGetCollectionElements(properties, &curDevice);
            return X52STD;
        case 0x762:
            HIDGetCollectionElements(properties, &curDevice);
            return X52PRO;
        case 0xBAC:
            HIDGetCollectionElements(properties, &curDevice);
            return YOKE;
        default:
            return OTHER;
    }
    return OTHER;
}

IOHIDDeviceInterface** X52::create_deviceinterface(io_object_t hidDevice)
{
    io_name_t className;
    IOCFPlugInInterface **plugInInterface = NULL;
    HRESULT plugInResult = S_OK;
    SInt32 score = 0;
    IOReturn ioReturnValue = kIOReturnSuccess;
    IOHIDDeviceInterface ** pphidDeviceInterface = NULL;
    
    ioReturnValue = IOObjectGetClass(hidDevice, className);
    if (ioReturnValue != kIOReturnSuccess)
    {
        printf("could not retrieve classname\n");
        return 0;
    }
    ioReturnValue = IOCreatePlugInInterfaceForService (hidDevice, kIOHIDDeviceUserClientTypeID,
                                                       kIOCFPlugInInterfaceID, &plugInInterface, &score);
    if (ioReturnValue == kIOReturnSuccess)
    {
        // Call a method of the intermediate plug-in to create the device interface
        plugInResult = (*plugInInterface)->QueryInterface (plugInInterface,
                                                           CFUUIDGetUUIDBytes (kIOHIDDeviceInterfaceID), (void**)(void*)&pphidDeviceInterface);
        if (plugInResult != S_OK)
            printf("Couldn’t query HID class device interface from plugInInterface\n");
        (*plugInInterface)->Release (plugInInterface);
    }
    else
        printf("Failed to create **plugInInterface via IOCreatePlugInInterfaceForService.\n");
    return pphidDeviceInterface;
}

void X52::event_interface(IOHIDDeviceInterface **hidDeviceInterface)
{
    HRESULT result;
    IOHIDQueueInterface ** queue;
    IOHIDEventStruct event;

    queue = (*hidDeviceInterface)->allocQueue (hidDeviceInterface);
    if (queue)
    {
        //create the queue
        result = (*queue)->create (queue, 0, /* flag?? */ 32);
        /* depth: maximum number of elements
         in queue before oldest elements in
         queue begin to be lost*/
        
        
        //add elements to the queue
        for (int i = 0; i < curDevice.buttons; ++i)
            (*queue)->addElement(queue, buttonElements[i], 0);
        
        CFRunLoopSourceRef tCFRunLoopSourceRef = NULL;

        result = (*queue)->createAsyncEventSource(queue, &tCFRunLoopSourceRef);
        if (kIOReturnSuccess != result)
            printf ("Failed to createAsyncEventSource, error: %ld.\n", result);
        
        // if we have one now…
        if (NULL != tCFRunLoopSourceRef)
        {
            CFRunLoopRef tCFRunLoopRef = (CFRunLoopRef) GetCFRunLoopFromEventLoop(GetMainEventLoop());
            
            // and it's not already attached to our runloop…
            if (!CFRunLoopContainsSource(tCFRunLoopRef, tCFRunLoopSourceRef, kCFRunLoopDefaultMode))
                // then attach it now.
                CFRunLoopAddSource(tCFRunLoopRef, tCFRunLoopSourceRef, kCFRunLoopDefaultMode);
        }
        result = (*queue)->start (queue);
        while (_isRunning)
        {
            AbsoluteTime zeroTime = {0,0};
            while( (result = (*queue)->getNextEvent (queue, &event, zeroTime, 0)) != kIOReturnUnderrun)
            {
                if (result != kIOReturnSuccess)
                    printf ("queue getNextEvent result error: %lx\n", result);
                else
                {
                   // printf ("queue: event:[0x%lX, 0x%lX] %ld\n", (unsigned long) event.elementCookie, buttonCookies[(int)event.elementCookie], event.value);
                    dispatch_input(buttonCookies[(int)event.elementCookie],  event.value);
                }
                fflush (stdout);
            }
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1f, true);
        }
        
        //stop data delivery to queue
        result = (*queue)->stop (queue);
        
        tCFRunLoopSourceRef = (*queue)->getAsyncEventSource(queue);
        if (NULL != tCFRunLoopSourceRef)	// if so then…
        {
            CFRunLoopRef tCFRunLoopRef = (CFRunLoopRef) GetCFRunLoopFromEventLoop(GetMainEventLoop());
            
            // if it's attached to our runloop…
            if (CFRunLoopContainsSource(tCFRunLoopRef, tCFRunLoopSourceRef, kCFRunLoopDefaultMode))
                // then remove it
                CFRunLoopRemoveSource(tCFRunLoopRef, tCFRunLoopSourceRef, kCFRunLoopDefaultMode);
            // now release it.
            CFRelease(tCFRunLoopSourceRef);
        }
        
        result = (*queue)->dispose (queue);
        result = (*queue)->Release (queue);
  		fflush (stdout);
    }
}


void X52::process_device(io_object_t hidDevice)
{
    IOHIDDeviceInterface** pphidDeviceInterface = 0;
    IOReturn ioReturnValue = kIOReturnSuccess;
    
    pphidDeviceInterface = create_deviceinterface(hidDevice);
    if (!pphidDeviceInterface)
        return;
    
    if (*pphidDeviceInterface)
    {
        ioReturnValue = (*pphidDeviceInterface)->open(pphidDeviceInterface, 0);
        
        event_interface(pphidDeviceInterface);
        
        if (ioReturnValue == KERN_SUCCESS)
        {
            ioReturnValue = (*pphidDeviceInterface)->close(pphidDeviceInterface);
        }
        (*pphidDeviceInterface)->Release(pphidDeviceInterface);
    }
    
    return;
}

int X52::mac_jsstart() {
    mach_port_t masterPort = 0;
    io_iterator_t hidObjectIterator = 0;
    IOReturn ioReturnValue = kIOReturnSuccess;
    io_object_t hidDevice = 0;
    
    ioReturnValue = IOMasterPort(bootstrap_port, &masterPort);
    if (ioReturnValue != kIOReturnSuccess)
    {
        printf("Couldn’t create a master I/O Kit Port.\n");
        return 1;
    }
    
    hidObjectIterator = find_device(masterPort, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
    if (!hidObjectIterator)
    {
        printf("no joysticks found\n");
        return 1;
    }
    
    while ((hidDevice = IOIteratorNext (hidObjectIterator)))
	{
		if (is_x52product(hidDevice) != OTHER)
        {
            printf("Saitek X52 product found.\n");
            process_device(hidDevice);
        }
        ioReturnValue = IOObjectRelease(hidDevice);
        if (ioReturnValue != kIOReturnSuccess)
        {
            printf("Couldn’t release HID device.\n");
            return 1;
        }
	}
	IOObjectRelease (hidObjectIterator);
    
    fflush(stdout);
    
    if (masterPort)
        mach_port_deallocate(mach_task_self (), masterPort);
    
    return 0;
}
#endif
