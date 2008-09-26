#include <sstream>
#include "XPLM/XPLMProcessing.h"
#include "XPLM/XPLMDataAccess.h"
#include "x52data.h"
#include "x52mfdpage.h"

using std::set;

x52data_t::x52data_t(void)
{
}

x52data_t::~x52data_t(void)
{
    for (set<x52object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        delete *it;
    }
    debug_out(info, "deleted data connection (self: %p)", this);
}

const float& x52data_t::upd_interval(void)
{
    return a_interval;
}

void x52data_t::connect(float interval)
{
    a_interval = interval;
    XPLMRegisterFlightLoopCallback(update, interval, this);
    debug_out(info, "engaged data connection (self: %p)", this);
}

void x52data_t::disconnect()
{
    // this is basically unnecessary, since X-Plane stops
    // callback execution of disabled plugins anyway, but
    // we want to be a bit paranoid here :-)
    XPLMUnregisterFlightLoopCallback(update, this);
    debug_out(info, "deactivating data connection (self: %p)", this);
}

void x52data_t::add_listener(x52listener_t* listener)
{
    if (!listener) return;
    if (a_listeners.insert(listener).second)
    {
        debug_out(info, "added listener: {%s} (self: %p)", listener->name().c_str(), listener);
    }
}

void x52data_t::remove_listener(x52listener_t* listener)
{
    if (!listener) return;
    if (a_listeners.erase(listener))
    {
        debug_out(info, "removed listener: {%s} (self: %p)", listener->name().c_str(), listener);
    }
}

// TODO: avoid adding redundant sources, currently the address is uses for
// identification, which is always unique
x52object_t* x52data_t::add_datasource(const char* ref)
{
    if (!ref) return 0;
    x52datasource_t* source = 0;
    try
    {
        source = new x52datasource_t(ref);
        if (a_datasources.insert(source).second)
            debug_out(info, "added datasource '%s' (self: %p)", source->name().c_str(), source);
    }
    catch (const char* reason)
    {
        debug_out(err, "%s (%s)", reason, ref);
        return 0;
    }
    return source;
}

void x52data_t::remove_datasource(x52object_t* source)
{
    if (!source) return;
    if (a_datasources.erase(source))
    {
        debug_out(info, "removed datasource '%s' (self: %p)", source->name().c_str(), source);
        delete source;
    }
}

void x52data_t::remove_datasource(const char* ref)
{
    if (!ref) return;
    for (set<x52object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        if ((*it)->name() == ref)
            remove_datasource(*it);
    }
}


/* private members */

void x52data_t::refresh_datasources(void)
{
    a_wantsupdate.clear();
    for (set<x52object_t*>::iterator it = a_datasources.begin(); it != a_datasources.end(); ++it)
    {
        if ((*it)->refresh())
        {
            debug_out(info, "'%s' reported new data (%s)", (*it)->name().c_str(), (const char*)*(*it));
            refresh_listeners(*it);
        }
    }
    for (set<x52listener_t*>::iterator it = a_wantsupdate.begin(); it != a_wantsupdate.end(); ++it)
    {
        (*it)->final_refresh();
    }
}

void x52data_t::refresh_listeners(x52object_t* updated_source)
{
    int not_orphaned = 0;
    for (set<x52listener_t*>::iterator it = a_listeners.begin(); it != a_listeners.end(); ++it)
    {
        if ((*it)->has_object(updated_source))
        {
            not_orphaned++;
            if ((*it)->refresh(updated_source))
                a_wantsupdate.insert(*it);
        }
    }
    if (!not_orphaned)
        remove_datasource(updated_source);
}

// static member, called by X-Plane
float x52data_t::update(float elapsed_lastcall, float elapsed_lastloop, int n_loop, void* arg)
{
    x52data_t* me = reinterpret_cast<x52data_t*>(arg);
    me->refresh_datasources();
    return me->upd_interval();
}


/*
** x52datasource_t
*/

x52datasource_t::x52datasource_t(const char* ref) : a_name(ref), a_newdata(true)
{
    a_ref = XPLMFindDataRef(ref);
    if (!a_ref)
        throw "data source not found";

    a_reftype = XPLMGetDataRefTypes(a_ref);
    if (!a_reftype)
        throw "data type not applicable";
    a_dint = 0;
    a_dfloat = 0;
    a_ddouble = 0;
    a_dstring.clear();
}

x52datasource_t::~x52datasource_t(void)
{
    debug_out(info, "deleted data source '%s' (self: %p)", a_name.c_str(), this);
}

const std::string& x52datasource_t::name(void)
{
    return a_name;
}

bool x52datasource_t::newdata_available(void)
{
    return a_newdata;
}

void x52datasource_t::mark_dirty(void)
{
    a_isdirty = true;
}

// we use standard casts for now, i.e. no exact rounding

x52datasource_t::operator int(void)
{
    switch (a_reftype)
    {
    case xplmType_Int:
        return a_dint;
    case xplmType_Float:
        return a_dfloat;
    case xplmType_Double:
        return a_ddouble;
    default:
        return 0;
    }
}

x52datasource_t::operator double(void)
{
    switch (a_reftype)
    {
    case xplmType_Int:
        return a_dint;
    case xplmType_Float:
        return a_dfloat;
    case xplmType_Double:
        return a_ddouble;
    default:
        return 0;
    }
}

x52datasource_t::operator float(void)
{
    switch (a_reftype)
    {
    case xplmType_Int:
        return a_dint;
    case xplmType_Float:
        return a_dfloat;
    case xplmType_Double:
        return a_ddouble;
    default:
        return 0;
    }
}

x52datasource_t::operator const std::string&(void)
{
    std::ostringstream os;
    switch (a_reftype)
    {
    case xplmType_Int:
        os << a_dint;
        break;
    case xplmType_Float:
        os << a_dfloat;
        break;
    case xplmType_Double:
        os << a_ddouble;
        break;
    }
    a_dstring.assign(os.str());
    return a_dstring;
}

x52datasource_t::operator const char*(void)
{
    std::ostringstream os;
    switch (a_reftype)
    {
    case xplmType_Int:
        os << a_dint;
        break;
    case xplmType_Float:
        os << a_dfloat;
        break;
    case xplmType_Double:
        os << a_ddouble;
        break;
    }
    a_dstring.assign(os.str());
    return a_dstring.c_str();
}

/* private members */

bool x52datasource_t::refresh(void)
{
    int   dint;
    float  dfloat;
    double  ddouble;
    a_newdata = false;
    switch (a_reftype)
    {
    case xplmType_Int:
        if (((dint = XPLMGetDatai(a_ref)) != a_dint) || a_isdirty)
        {
            a_dint = dint;
            a_isdirty = false;
            return (a_newdata = true);
        }
        else return false;
    case xplmType_Float:
        if (((dfloat = XPLMGetDataf(a_ref)) != a_dfloat) || a_isdirty)
        {
            a_dfloat = dfloat;
            a_isdirty = false;
            return (a_newdata = true);
        }
        else return false;
    case xplmType_Double:
        if (((ddouble = XPLMGetDatad(a_ref)) != a_ddouble) || a_isdirty)
        {
            a_ddouble = ddouble;
            a_isdirty = false;
            return (a_newdata = true);
        }
        else return false;
    case xplmType_FloatArray:
        return false;
    case xplmType_IntArray:
        return false;
    case xplmType_Data:
        return false;
    case xplmType_Unknown:
        return false;
    default:
        return false;
    }
}
