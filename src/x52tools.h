#ifndef X52TOOLS_H
#define X52TOOLS_H

#include <string>

class x52tools_t
{
public:
    x52tools_t(void);
    void debug_out(int type, const char* msg, ...);
    static void debug_out(const char* msg, ...);
    bool verbose;
    enum dbgtype_e
    {
        info,
        warn,
        err
    };
private:
};

class x52object_t
    {};

class x52listener_t
{
public:
    virtual const std::string& name(void)=0;
private:
    virtual bool has_object(x52object_t* obj)=0;
    virtual bool refresh(x52object_t* obj)=0;
    virtual void final_refresh(void)=0;
};


#endif /* X52TOOLS_H */
