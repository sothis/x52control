#ifndef X52DATA_H
#define X52DATA_H

#include <set>
#include <string>
#include "x52tools.h"

class x52mfdpage_t;

class x52datasource_t : public x52object_t, public x52tools_t
{
public:
    x52datasource_t(const char* ref);
    ~x52datasource_t(void);

    const std::string& name(void);
    bool newdata_available(void);
    void mark_dirty(void);
    operator int(void);
    operator float(void);
    operator double(void);
    operator const std::string&(void);
    operator const char*(void);
    bool refresh(void);
private:
    std::string a_name;
    void*  a_ref;
    int   a_reftype;
    bool  a_newdata;
    bool  a_isdirty;
    int   a_dint;
    float  a_dfloat;
    double  a_ddouble;
    std::string a_dstring;
};

class x52data_t : public x52provider_t, public x52tools_t
{
public:
    x52data_t(void);
    ~x52data_t(void);

    void add_listener(x52listener_t* listener);
    void remove_listener(x52listener_t* listener);
    x52object_t* add_datasource(const char* ref);
    void remove_datasource(x52object_t* source);
    void remove_datasource(const char* ref);
    void connect(float interval);
    void disconnect();
    const float& upd_interval(void);

private:
    static float update(float elapsed_lastcall, float elapsed_lastloop, int n_loop, void* arg);
    void refresh_datasources(void);
    void refresh_listeners(x52object_t* updated_source);
    std::set<x52listener_t*> a_listeners;
    std::set<x52object_t*> a_datasources;
    std::set<x52listener_t*> a_wantsupdate;
    float a_interval;
};

#endif /* X52DATA_H */
