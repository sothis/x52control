#include <string.h>
#include <stdlib.h>
#include "XPLM/XPLMDefs.h"
#include "x52session.h"

const char* version = VERSION;

#if IBM
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
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

static x52session_t* x52session = 0;

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
        x52session = new x52session_t();
    }
    catch (const char* e)
    {
        if (!strcmp(e, "x52control.nonfatal"))
            return 0;
        else
            abort();
    }
    return 1;
}

PLUGIN_API int XPluginEnable(void)
{
    if (!x52session) return 0;
    return x52session->enable();
}

PLUGIN_API void XPluginDisable(void)
{
    if (!x52session) return;
    x52session->disable();
    return;
}

PLUGIN_API void XPluginStop(void)
{
    if (x52session) delete x52session;
    return;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void* inParam)
{
    return;
}
