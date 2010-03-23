#include "x52session.h"

#include <stdint.h>
#include <string.h>

#define X52PLUGINNAME_STR	"x52control [v" VERSION "]"
#define X52SIGNATURE_STR	"copai.equipment.x52control"
#define X52DESCRIPTION_STR	"Saitek X52 and X52Pro interface plugin, " \
				"compiled on " __DATE__ ". Send feedback " \
				"to janos.dev@gmail.com."


extern void init_ctor();

__attribute__ ((visibility("default"))) int
XPluginStart(char* name, char* sig, char* descr)
{
	init_ctor();
	strcpy(name, X52PLUGINNAME_STR);
	strcpy(sig, X52SIGNATURE_STR);
	strcpy(descr, X52DESCRIPTION_STR);
	return 1;
}

__attribute__ ((visibility("default"))) int
XPluginEnable(void)
{
	return x52s_enable();
}

__attribute__ ((visibility("default"))) void
XPluginDisable(void)
{
	x52s_disable();
}

__attribute__ ((visibility("default"))) void
XPluginStop(void)
{
	x52s_disable();
}

__attribute__ ((visibility("default"))) void
XPluginReceiveMessage(uint32_t from, uint32_t msg, void* arg)
{}

