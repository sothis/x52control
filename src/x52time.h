#ifndef X52TIME_H
#define X52TIME_H

#include <map>
#include <string>
#include <time.h>
#include "x52tools.h"

class x52out_t;
class x52data_t;
class x52datasource_t;

class x52time_t : public x52listener_t, public x52tools_t
{
public:
    x52time_t(const char* name, x52out_t* outputdevice, x52data_t* dataconnection);
    ~x52time_t(void);

    const std::string& name(void);
    void set_datasources(std::map<int, std::string>* sources);
    void set_active(bool active);
    bool has_object(x52object_t* source);
    bool refresh(x52object_t* source);
    void final_refresh(void);
    int x52led_debug;

private:
    bool a_visible;
    int a_gear;
    int a_autopilot_engaged;
    int a_weapons_armed;
    char a_hours;
    char a_mins;
    char a_lastmin;
    int  a_secs;
    time_t a_time;
    struct tm a_tr;
    x52out_t* a_outdevice;
    x52data_t* a_dataconnection;
    std::string a_data;
    std::string a_name;
    std::string a_template;
    std::map<int, x52object_t*> a_datasources;
};

#endif /* X52TIME_H */
