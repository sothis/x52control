#include <string.h>
#include "x52out.h"
#include "x52data.h"
#include "x52time.h"

using std::map;
using std::string;

x52time_t::x52time_t(const char* name, x52out_t* outputdevice, x52data_t* dataconnection)
        : a_visible(false), a_outdevice(outputdevice), a_dataconnection(dataconnection)
{
    a_hours = 0;
    a_mins = 0;
    a_secs = 0;
    // force update when refreshed the first time
    a_lastmin = 127;
    if (!name || !strlen(name))
        a_name.assign(" -unnamed datalistener- ");
    else
        a_name.assign(name);
    a_dataconnection->add_listener(this);

    map<int, string> sources;
    if (a_name == "std.time.x-plane")
    {
        sources[0] = "sim/time/local_time_sec";
        sources[1] = "sim/time/zulu_time_sec";
        set_datasources(&sources);
    }
    a_time = time(NULL);
    localtime_r(&a_time, &a_tr);
    // initialize with current date and 00:00:00
    final_refresh();
}

x52time_t::~x52time_t(void)
{
    a_dataconnection->remove_listener(this);
    debug_out(info, "deleted timer {%s} (self: %p)", a_name.c_str(), this);
}

const std::string& x52time_t::name(void)
{
    return a_name;
}

void x52time_t::set_active(bool active)
{
    a_visible = active;
    if (active)
        for (map<int, x52object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
        {
            (*it).second->mark_dirty();
        }
}

void x52time_t::set_datasources(std::map<int, std::string>* sources)
{
    if (!sources) return;
    a_datasources.clear();
    for (map<int, string>::iterator it = sources->begin(); it != sources->end(); ++it)
    {
        x52object_t* src;
        src = a_dataconnection->add_datasource((*it).second.c_str());
        if (src) a_datasources[(*it).first] = src;
    }
}

bool x52time_t::has_object(x52object_t* source)
{
    for (map<int, x52object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        if ((*it).second == source)
            return true;
    }
    return false;
}

bool x52time_t::refresh(x52object_t* source)
{
    if (!a_visible) return false;

    if (source->name() == "sim/time/local_time_sec")
    {
        float currsecs = *source;
        a_secs = currsecs;
        a_hours = a_secs/3600;
        a_mins = (a_secs%3600)/60;
        a_secs = (a_secs%3600)%60;
        if (a_mins != a_lastmin)
        {
            a_lastmin = a_mins;
            a_time = time(NULL);
            localtime_r(&a_time, &a_tr);
            return true;
        }
    }
    return false;
}

void x52time_t::final_refresh(void)
{
    debug_out(info, "updating joystick time (%s)", a_name.c_str());
    a_outdevice->time(true, a_hours, a_mins);
    a_outdevice->date(a_tr.tm_year+1900, a_tr.tm_mon+1, a_tr.tm_mday);
}
