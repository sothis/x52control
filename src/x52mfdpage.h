#ifndef X52MFDPAGE_H
#define X52MFDPAGE_H

#include <map>
#include <string>
#include "x52tools.h"

class x52out_t;
class x52data_t;
class x52datasource_t;

class x52mfdpage_t : public x52listener_t, public x52tools_t
{
public:
    x52mfdpage_t(const char* name, x52out_t* outputdevice, x52data_t* dataconnection);
    ~x52mfdpage_t(void);

    const std::string& name(void);
    void set_datasources(std::map<int, std::string>* sources);
    void set_active(bool active);
    bool has_object(x52object_t* source);
    bool refresh(x52object_t* source);
    void final_refresh(void);
private:
    bool a_visible;
    x52out_t* a_outdevice;
    x52data_t* a_dataconnection;
    std::string a_data;
    std::string a_name;
    std::string a_template;
    std::map<int, x52object_t*> a_datasources;
};

#endif /* X52MFDPAGE_H */
