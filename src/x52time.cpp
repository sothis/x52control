#include <string.h>
#include <fstream>
#include <stdlib.h>
#include "x52out.h"
#include "x52data.h"
#include "x52time.h"

using namespace std;
using std::map;
using std::string;

x52time_t::x52time_t(const char* name, x52out_t* outputdevice, x52data_t* dataconnection)
        : a_visible(false), a_outdevice(outputdevice), a_dataconnection(dataconnection)
{
    a_gear = 0;
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
        int count=2;
        read_config();
        map<string,string> config=x52control_config;
        //add datarefs from config file
        for (map<string, string>::iterator it = config.begin(); it != config.end(); ++it) {
            //debug_out(warn, "configuration map: %s = '%s'", it->first.c_str(), it->second.c_str());
            int dataref=it->first.find("sim/",0);
            if (dataref >= 0 ) {
                //real dataref
                sources[count++]=it->first.c_str();
                //debug_out(warn, "added dataref: %s", it->first.c_str());
            } else {
                //predefined or settings
                //debug_out(warn, "setting or predefined: %s",it->first.c_str());
                if (it->first ==  "brightness_mfd") {
                    a_outdevice->display_brightness(atoi(it->second.c_str()));
                } else if (it->first ==  "brightness_led") {
                    a_outdevice->led_brightness(atoi(it->second.c_str()));
                } else if (it->first ==  "debug") {
                    x52led_debug=atoi(it->first.c_str());
                } else if (it->first ==  "gear") {
                     sources[count++]="sim/cockpit/switches/gear_handle_status";
                     sources[count++]="sim/cockpit/warnings/annunciators/gear_unsafe";
                } else if (it->first == "autopilot") {
                     sources[count++]="sim/cockpit/autopilot/autopilot_mode";
                     sources[count++]="sim/cockpit/warnings/annunciators/autopilot_disconnect";
                } else if (it->first ==  "fire") {
                     sources[count++]="sim/cockpit/weapons/guns_armed";
                     sources[count++]="sim/cockpit/weapons/rockets_armed";
                     sources[count++]="sim/cockpit/weapons/missiles_armed";
                     sources[count++]="sim/cockpit/weapons/bombs_armed";
                } else {
                    debug_out(err,"unknown setting or predefined configuration item: %s", it->first.c_str());
                }
            }
        }
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
    for (map<int, x52object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)    {
        if ((*it).second == source )
            return true;
    }
    return false;
}

bool x52time_t::refresh(x52object_t* source)
{
    string action;
    map<string,string> config=x52control_config;
    x52led_debug=atoi(config["debug"].c_str());
    if (!a_visible) return false;

    if (source->name() == "sim/time/local_time_sec")
    {
        float currsecs = *source;
        a_secs = (int) currsecs;
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
    } else if (source->name() == "sim/time/zulu_time_sec") {
        (void)0;
    } else if (source->name() == "sim/cockpit/switches/gear_handle_status" ) {
        //keep state
        a_gear = (int) *source;
        return true;
    } else if (source->name() == "sim/cockpit/warnings/annunciators/gear_unsafe" ) {
        //go to orange if unsafe, go to gear handle state otherwise
        //do we have a predefined action for gear?
        try {
            action = config["gear"];
        } catch (const char* reason) {
            return true;
        }
        int i = *source;
        if (i != 0) {
            //gear not safe turn orange
            action+=",orange";
            a_outdevice->set_led_by_name(x52led_debug,source->name().c_str(),0,action.c_str());
        } else {
            //gear now safe
            if (a_gear == 0) {
                //retracted
                action+=",off";
                a_outdevice->set_led_by_name(x52led_debug,source->name().c_str(),0,action.c_str());
            } else {
                //extended
                action+=",green";
                a_outdevice->set_led_by_name(x52led_debug,source->name().c_str(),0,action.c_str());
            }
        }
        return true;
    } else if (source->name() == "sim/cockpit/autopilot/autopilot_mode") {
        int i = (int) *source;
        try {
            action = config["autopilot"];
        } catch (const char* reason) {
            return true;
        }
        a_autopilot_engaged=i;
        action+=",off,orange,green";
        a_outdevice->set_led_by_name(x52led_debug,source->name().c_str(),i,action.c_str());
        return true;
    } else if (source->name() == "sim/cockpit/warnings/annunciators/autopilot_disconnect") {
        int i = (int) *source;
        try {
            action = config["autopilot"];
        } catch (const char* reason) {
            return true;
        }
        if (i == 1 ) {
            action+=",red";
            a_outdevice->set_led_by_name(x52led_debug,source->name().c_str(),0,action.c_str());
        } else {
            action+=",off,orange,green";
            a_outdevice->set_led_by_name(x52led_debug,source->name().c_str(),a_autopilot_engaged,action.c_str());
        }
        return true;
    } else if (source->name() == "sim/cockpit/weapons/guns_armed" ||
            source->name() == "sim/cockpit/weapons/rockets_armed" ||
            source->name() == "sim/cockpit/weapons/missiles_armed" ||
            source->name() == "sim/cockpit/weapons/bombs_armed") {
        int i = (int) *source;
        action = config["fire"];
        action+=",off,on";
        a_outdevice->set_led_by_name(x52led_debug,source->name().c_str(),i,action.c_str());
    } else {
        //unhandled or custom action from config file
        try {
            action = config[source->name()];
            int i = (int) *source;
            a_outdevice->set_led_by_name(x52led_debug,source->name().c_str(),i,action.c_str());
        } catch (const char* e) {
            //unhandled data refs
            //use this to check on registered datarefs that have changed
            float f = *source;
            debug_out(warn,"unhandled data ref: %s: %6.2f", source->name().c_str(),f);
        }
        /*
        float f = *source;
        debug_out(err,"unhandled data ref: %s: %6.2f", source->name().c_str(),f);
        */
        return true;
    }

    return false;
}

void x52time_t::final_refresh(void)
{
    debug_out(info, "updating joystick time (%s)", a_name.c_str());
    a_outdevice->time(true, a_hours, a_mins);
    a_outdevice->date(a_tr.tm_year+1900, a_tr.tm_mon+1, a_tr.tm_mday);
}
