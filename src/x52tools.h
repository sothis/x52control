#ifndef X52TOOLS_H
#define X52TOOLS_H

#include <string>
#include <map>

using namespace std;

class x52tools_t
{
public:
    x52tools_t(void);
    void debug_out(int type, const char* msg, ...);
    static void debug_out(const char* msg, ...);
    void read_config();
    bool verbose;
    enum dbgtype_e
    {
        info,
        warn,
        err
    };
    std::map<std::string, std::string> x52control_config;
private:
};

class x52object_t
{
public:
    virtual ~x52object_t(void){};
    virtual const std::string& name(void)=0;
    virtual bool refresh(void)=0;
    virtual void mark_dirty(void)=0;
    virtual operator int(void)=0;
    virtual operator float(void)=0;
    virtual operator double(void)=0;
    virtual operator const std::string&(void)=0;
    virtual operator const char*(void)=0;
};

class x52provider_t
{
public:
    virtual ~x52provider_t(void){};
private:
private:
    virtual void refresh_datasources(void)=0;
    virtual void refresh_listeners(x52object_t* updated_source)=0;
};

class x52listener_t
{
public:
    virtual ~x52listener_t(void){};
    virtual const std::string& name(void)=0;
    virtual bool has_object(x52object_t* obj)=0;
    virtual bool refresh(x52object_t* obj)=0;
    virtual void final_refresh(void)=0;
};


#endif /* X52TOOLS_H */
