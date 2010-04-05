#include <string.h>
#include <time.h>
#include "x52out.h"
#include "x52data.h"
#include "x52mfdpage.h"

using std::map;
using std::string;

x52mfdpage_t::x52mfdpage_t(const char* name, x52out_t* outputdevice, x52data_t* dataconnection)
        : a_visible(false), a_outdevice(outputdevice), a_dataconnection(dataconnection)
{
    if (!name || !strlen(name))
        a_name.assign(" -unnamed page- ");
    else
        a_name.assign(name);
    a_dataconnection->add_listener(this);

// handle some default cases

    map<int, string> sources;
    if (a_name == "std.radio.com")
    {
        sources[0] = "sim/cockpit/radios/com1_freq_hz";
        sources[1] = "sim/cockpit/radios/com2_freq_hz";
        sources[2] = "sim/cockpit/radios/com1_stdby_freq_hz";
        sources[3] = "sim/cockpit/radios/com2_stdby_freq_hz";
        a_template.assign(" Com1      Com2\n%%06.2f Ac %%06.2f\n%%06.2f Sb %%06.2f");
        set_datasources(&sources);
    }
    if (a_name == "std.radio.nav")
    {
        sources[0] = "sim/cockpit/radios/nav1_freq_hz";
        sources[1] = "sim/cockpit/radios/nav2_freq_hz";
        sources[2] = "sim/cockpit/radios/nav1_stdby_freq_hz";
        sources[3] = "sim/cockpit/radios/nav2_stdby_freq_hz";
        a_template.assign(" Nav1      Nav2\n%%06.2f Ac %%06.2f\n%%06.2f Sb %%06.2f");
        set_datasources(&sources);
    }
    if (a_name == "std.radio.adf")
    {
        sources[0] = "sim/cockpit/radios/adf1_freq_hz";
        sources[1] = "sim/cockpit/radios/adf2_freq_hz";
        sources[2] = "sim/cockpit/radios/adf1_stdby_freq_hz";
        sources[3] = "sim/cockpit/radios/adf2_stdby_freq_hz";
        a_template.assign(" Adf1      Adf2\n%%03d Hz Ac %%03d Hz\n%%03d Hz Sb %%03d Hz");
        set_datasources(&sources);
    }
    if (a_name == "navigation_1")
    {
        sources[0] = "sim/flightmodel/position/psi";
        sources[1] = "sim/flightmodel/position/indicated_airspeed2";
        sources[2] = "sim/flightmodel/misc/h_ind";
        sources[3] = "sim/flightmodel/position/vh_ind_fpm2";
        a_template.assign("hdg:   %%03d\nspd:  %%4d\nfl :   %%3d %%4d");
        set_datasources(&sources);
    }
}

x52mfdpage_t::~x52mfdpage_t(void)
{
    a_dataconnection->remove_listener(this);
    debug_out(info, "deleted mfd page {%s} (self: %p)", a_name.c_str(), this);
}

const std::string& x52mfdpage_t::name(void)
{
    return a_name;
}

void x52mfdpage_t::set_active(bool active)
{
    a_visible = active;
    if (active)
        for (map<int, x52object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
        {
            (*it).second->mark_dirty();
        }
}

void x52mfdpage_t::set_datasources(std::map<int, std::string>* sources)
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

bool x52mfdpage_t::has_object(x52object_t* source)
{
    for (map<int, x52object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        if ((*it).second == source)
            return true;
    }
    return false;
}

bool x52mfdpage_t::refresh(x52object_t* source)
{
    if (!a_visible) return false;
    return true;
}

void x52mfdpage_t::final_refresh(void)
{
    debug_out(info, "refreshing joystick display (%s)", a_name.c_str());
    // handle some default cases - VERY DIRTY HAX, big fixme here

    char temp[2048] = {};
    size_t p = std::string::npos;
    string t2 = a_template.c_str();
    int i;
    float f;
    int count;
    count=0;

    if (a_name == "navigation_1") {
            time_t seconds;
            seconds=time(NULL);
            if ( last_page_update == seconds )
                return;
            last_page_update=seconds;
    }

    for (map<int, x52object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        count+=1;
        if ((p = t2.find('%')) != std::string::npos)
        {
            t2.erase(p, 1);
        }
        memset(temp, 0, 2048);
        if (a_name == "navigation_1") {
            if (count <=2) {
                i = (int)*a_datasources[(*it).first];
            } else {
                i = (int)*a_datasources[(*it).first]/100;
            }
            snprintf(temp, 2048, t2.c_str(), i);
        } else if (a_name == "std.radio.adf") {
            i = (int)*a_datasources[(*it).first];
            snprintf(temp, 2048, t2.c_str(), i);
        } else {
            //now "std.radio.com" or "std.radio.nav"
            f = (int)*a_datasources[(*it).first]/100.0f;
            snprintf(temp, 2048, t2.c_str(), f);
        }

        t2 = temp;
        p = 0;
        while ((p = t2.find('%', p)) != std::string::npos)
        {
            t2.replace(p, 1, "%%");
            p += 2;
        }
    }
    a_data = temp;
    a_outdevice->set_textdata(a_data.c_str());
    a_outdevice->print();
}
