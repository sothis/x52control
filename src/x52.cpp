#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#if IBM
#include <io.h>
#define TEMP_FAILURE_RETRY(x) x
#define snprintf sprintf_s
#endif
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
        printf("data source not found.\n");
        return;
    }
    reftype = XPLMGetDataRefTypes(temp);
    if (!reftype)
    {
        printf("data type not applicable\n");
        return;
    }
    if (_mfd_mutex) pthread_mutex_lock(_mfd_mutex);
    datasources[source] = std::make_pair(temp, reftype);
    if (_mfd_mutex) pthread_mutex_unlock(_mfd_mutex);
    printf("added data source: %s\n", source.c_str());
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
        // we won't join the input thread, since read
        // blocks, and select is teh suck, maybe trying poll()?
        //pthread_join(_ptInput, NULL);
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
#if LIN
    struct js_event jse;

    printf("X52 Input thread started\n");
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
