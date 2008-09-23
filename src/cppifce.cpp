#include <stdio.h>
#include <string.h>
#include "XPLM/XPLMProcessing.h"
#include "cppifce.h"
#include "output.h"

static x52out_t* x52out;

int cpp_plugin_start(void)
{
	try
	{
		x52out = new x52out_t();
	}
	catch (const char* reason)
	{
		printf("\033[0;31m[x52control]\033[0m: %s\n", reason);
		return 0;
	}
	catch(...)
	{
		printf("\\033[0;31m[x52control]\\033[0m: unhandled exception.\n");
		return 0;
	}
    return 1;
}

void cpp_plugin_stop(void)
{
	if (x52out) delete x52out;
    return;
}

int cpp_plugin_enable(void)
{
	if (!x52out) return 0;
	out_param_t test;
	memset(&test, 0, sizeof(out_param_t));
	strncpy(test.text, "Hello World\n12345\n67890", 50);
	test.next_call = 1.0f;
	x52out->set_verbose(true);
	XPLMRegisterFlightLoopCallback(x52out_t::update, 0.0f, x52out);
	x52out->refresh(&test);
    return 1;
}

void cpp_plugin_disable(void)
{
	if (x52out) XPLMUnregisterFlightLoopCallback(x52out_t::update, x52out);
    return;
}

void cpp_plugin_recvmsg(int from, long msg, void* param)
{
    return;
}
