//#include <sys/time.h>
#include "XPLM/XPLMUtilities.h"
#include "x52mfdpage.h"
#include "x52in.h"

using std::set;

x52in_t::x52in_t(void) : a_currentpage(0)
{
	a_cmd_pagecycle = XPLMCreateCommand("sim/instruments/x52_datapage_cycle", "cycles through datapages on the X52 display");
	if (!a_cmd_pagecycle)
		throw "wasn't able to create custom X-Plane commands";
	XPLMRegisterCommandHandler(a_cmd_pagecycle, dispatch_command, 0, this);
}

x52in_t::~x52in_t(void)
{
	for (set<x52mfdpage_t*>::iterator it = a_pages.begin(); it != a_pages.end(); ++it)
	{
		delete *it;
	}
	debug_out(info, "deleted joystick input handler (self: %p)", this);
}

void x52in_t::add_page(x52mfdpage_t* page)
{
	if (!page) return;
	if(a_pages.insert(page).second)
	{
		a_currentpage = page;
		debug_out(info, "added page to inputhandler: {%s} (self: %p)", page->name().c_str(), page);
	}
}

void x52in_t::handle_pagecycle(void)
{
	set<x52mfdpage_t*>::iterator it = a_pages.find(a_currentpage);
	if (it == a_pages.end())
		return;
    ++it;
    if (it == a_pages.end())
		it = a_pages.begin();
	a_currentpage->set_active(false);
    a_currentpage = *it;
	a_currentpage->set_active(true);

	debug_out(info, "switched datapage to: {%s}", a_currentpage->name().c_str());
}

int x52in_t::dispatch_command(XPLMCommandRef cmd, XPLMCommandPhase phase, void *arg)
{
	x52in_t* me = reinterpret_cast<x52in_t*>(arg);
	//struct timeval s;
	//static unsigned long ms_s, ms_e, ms_d;
	//struct timeval e;

	if (cmd == me->a_cmd_pagecycle)
			if (phase == xplm_CommandBegin) me->handle_pagecycle();
	return 0;
}
