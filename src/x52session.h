#ifndef X52SESSION_H
#define X52SESSION_H

#include "x52tools.h"

class x52out_t;
class x52in_t;
class x52data_t;
class x52time_t;

class x52session_t : public x52tools_t
{
public:
    x52session_t(void);
    ~x52session_t(void);
    int enable(void);
    void disable(void);
private:
    void create_defaultpages(void);
    x52out_t* a_joystickout;
    x52in_t* a_joystickin;
    x52data_t* a_dataconnection;
    x52time_t* a_x52time;
};

#endif /* X52SESSION_H */
