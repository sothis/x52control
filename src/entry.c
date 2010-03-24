#include "x52session.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define X52PLUGINNAME_STR	"x52control [v" VERSION "]"
#define X52SIGNATURE_STR	"copai.equipment.x52control"
#define X52DESCRIPTION_STR	"Saitek X52 and X52Pro interface plugin, " \
				"compiled on " __DATE__ ". Send feedback " \
				"to janos.dev@gmail.com."

enum init_constants_e {
	update_interval_ms = 200,
};

extern void init_ctor(void(*fn)(void));

typedef float (*floopfn)(float, float, int, void*);
extern void XPLMRegisterFlightLoopCallback(floopfn, float, void*);
extern void XPLMUnregisterFlightLoopCallback(floopfn, void*);

extern void* control_init(void);
extern void control_update(void* arg);
extern void control_end(void* arg);

static void* current_arg = 0;

static float
try_update(float elapsed_lastcall, float elapsed_lastloop, int loop, void* arg)
{
	control_update(arg);
	return (update_interval_ms/1000.0f);
}

static void init_connection(void)
{
	XPLMRegisterFlightLoopCallback(try_update,
		(update_interval_ms/1000.0f), current_arg);
}

static void close_connection(void)
{
	control_end(current_arg);
	x52s_disable(0);
}

__attribute__ ((visibility("default"))) int
XPluginStart(char* name, char* sig, char* descr)
{
	init_ctor(close_connection);
	strcpy(name, X52PLUGINNAME_STR);
	strcpy(sig, X52SIGNATURE_STR);
	strcpy(descr, X52DESCRIPTION_STR);
	current_arg = control_init();
	return 1;
}

__attribute__ ((visibility("default"))) int
XPluginEnable(void)
{
	return x52s_enable(init_connection);
}

/* we clean up resources using atexit() via init_ctor() */
__attribute__ ((visibility("default"))) void
XPluginDisable(void)
{
	XPLMUnregisterFlightLoopCallback(try_update, current_arg);
}

__attribute__ ((visibility("default"))) void
XPluginStop(void)
{}

__attribute__ ((visibility("default"))) void
XPluginReceiveMessage(uint32_t from, uint32_t msg, void* arg)
{}

