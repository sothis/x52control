#include <string.h>
#include <stdio.h>
#include "XPLM/XPLMProcessing.h"
#include "x52out.h"
#include "x52data.h"

using std::set;

x52data_t::x52data_t(void)
{
	//memset(&current_data, 0, sizeof(out_param_t));
}

x52data_t::~x52data_t(void)
{
}

float x52data_t::update(float elapsed_lastcall, float elapsed_lastloop, int n_loop, void* arg)
{
	x52data_t* me = reinterpret_cast<x52data_t*>(arg);

	me->current_state().next_call = 0;
	sprintf(me->current_state().text, "current loop:\n%d", n_loop);
	me->refresh_listeners();
	return me->upd_interval();
}

out_param_t& x52data_t::current_state(void)
{
	return current_data;
}

const float& x52data_t::upd_interval(void)
{
	return a_interval;
}

void x52data_t::engage(float interval)
{
	a_interval = interval;
	XPLMRegisterFlightLoopCallback(x52data_t::update, interval, this);
	debug_out(info, "engaged data connection %p", this);
}

void x52data_t::stop()
{
	XPLMUnregisterFlightLoopCallback(x52data_t::update, this);
	debug_out(info, "deactivating data connection %p", this);
}

void x52data_t::add_listener(x52listener_t* listener)
{
	if (!listener) return;
	if(a_listeners.insert(listener).second)
	{
		debug_out(info, "added listener %p", listener);
		XPLMRegisterFlightLoopCallback(x52out_t::update, 0.0f, listener);
	}
}

void x52data_t::remove_listener(x52listener_t* listener)
{
	if (!listener) return;
	if (a_listeners.erase(listener))
	{
		debug_out(info, "removed listener %p", listener);
		XPLMUnregisterFlightLoopCallback(x52out_t::update, listener);
	}
}

void x52data_t::refresh_listeners(void)
{
	for (set<x52listener_t*>::iterator it = a_listeners.begin(); it != a_listeners.end(); ++it)
	{
		(*it)->refresh(&current_data);
		XPLMSetFlightLoopCallbackInterval(x52out_t::update, -1.0f, 1, *it);
	}
}

