#define XPML200

#include <XPLM/XPLMDefs.h>
#include <string.h>
#include "cppifce.h"

#if IBM
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

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
    strcpy(outName, "x52control");
    strcpy(outSig, "copai.equipment.x52control");

    return cpp_plugin_start(outDesc);
}

PLUGIN_API void XPluginStop(void)
{
    cpp_plugin_stop();
    return;
}

PLUGIN_API void XPluginDisable(void)
{
    cpp_plugin_disable();
    return;
}

PLUGIN_API int XPluginEnable(void)
{
    return cpp_plugin_enable();
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void* inParam)
{
    cpp_plugin_recvmsg(inFromWho, inMessage, inParam);
    return;
}
