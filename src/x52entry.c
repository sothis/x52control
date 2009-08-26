#include "x52version.h"
#include "x52session.h"
#include <string.h>

__attribute__ ((visibility("default"))) int
XPluginStart(char* name, char* sig, char* descr)
{
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

