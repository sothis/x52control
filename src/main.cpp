#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "XPLM/XPLMDefs.h"
#include "x52out.h"
#include "x52data.h"

const char* version = "0.3.0";

#if IBM
#include <windows.h>

#ifdef __cplusplus
	extern "C" {
#endif
	PLUGIN_API int XPluginStart(char* name, char* signature, char* description);
	PLUGIN_API int XPluginEnable(void);
	PLUGIN_API void XPluginDisable(void);
	PLUGIN_API void XPluginStop(void);
	PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void* inParam);
#ifdef __cplusplus
}
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

static x52out_t* x52out = 0;
static x52data_t* x52data = 0;

PLUGIN_API int XPluginStart(char* name, char* signature, char* description)
{
	strcpy(name, "x52control (");
	strcat(name, version);
	strcat(name, ")");
	strcpy(signature, "copai.equipment.x52control");
	strcpy(description, "provides additional features for the Saitek X52 products, compiled on ");
	strcat(description, __DATE__);
	try
	{
		x52out = new x52out_t();
		x52data = new x52data_t();
	}
	catch (const char* reason)
	{
		printf("\033[0;31m[x52control]\033[0m: %s\n", reason);
		return 0;
	}
	catch(...)
	{
		printf("\033[0;31m[x52control]\033[0m: unhandled exception.\n");
		abort();
	}
    return 1;
}

PLUGIN_API int XPluginEnable(void)
{
	if (!x52out || !x52data) return 0;
	x52data->add_listener(x52out);
	x52data->engage(0.1f);
    return 1;
}

PLUGIN_API void XPluginDisable(void)
{
	if (!x52out || !x52data) return;
	x52data->remove_listener(x52out);
	x52data->stop();
	return;
}

PLUGIN_API void XPluginStop(void)
{
	if (x52data) delete x52data;
	if (x52data) delete x52out;
	return;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void* inParam)
{
	return;
}

