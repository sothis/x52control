#include "x52out.h"
#include "x52in.h"
#include "x52data.h"
#include "x52mfdpage.h"
#include "x52time.h"
#include "x52session.h"

x52session_t::x52session_t(void) : a_joystickout(0), a_joystickin(0), a_dataconnection(0)
{
    try
    {
        a_joystickout = new x52out_t();
        a_joystickin = new x52in_t();
    }
    catch (const char* reason)
    {
        debug_out(err, "could not create x52session (%s)", reason);
        if (a_joystickout) delete a_joystickout;
        throw "x52control.nonfatal";
    }
    a_dataconnection = new x52data_t();
    create_defaultpages();
}

x52session_t::~x52session_t(void)
{
    // fixme: currently x52in_t needs to be deleted before x52data_t
    // because the destructor relies on the existance of a x52data_t object
    // in conjuction with this fix also delete x52time_t objects
    a_dataconnection->remove_listener(a_x52time);
    delete a_x52time;
    delete a_joystickin;
    delete a_dataconnection;
    delete a_joystickout;
}

void x52session_t::create_defaultpages(void)
{
    a_joystickin->add_page(new x52mfdpage_t("std.radio.com", a_joystickout, a_dataconnection));
    a_joystickin->add_page(new x52mfdpage_t("std.radio.nav", a_joystickout, a_dataconnection));
    a_joystickin->add_page(new x52mfdpage_t("std.radio.adf", a_joystickout, a_dataconnection));
    a_x52time = new x52time_t("std.time.x-plane", a_joystickout, a_dataconnection);
    a_x52time->set_active(true);
}

int x52session_t::enable(void)
{
    a_dataconnection->connect(0.1f);
    return 1;
}

void x52session_t::disable(void)
{
    a_dataconnection->disconnect();
}
