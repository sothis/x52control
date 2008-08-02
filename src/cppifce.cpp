#include <string.h>
#include <stdio.h>
#include <XPLM/XPLMPlugin.h>
#include "cppifce.h"
#include "x52.h"

static X52* x52pro = NULL;

int cpp_plugin_start(char* outDesc)
{
    x52pro = new X52();
    if (!x52pro)
    {
        return 0;
    }
    if (!x52pro->isConnected())
    {
        delete x52pro;
        return 0;
    }
    strcpy(outDesc, "Integrates the Saitek X52 Pro X-Plane");
    return 1;
}

void cpp_plugin_stop(void)
{
    if (x52pro) delete x52pro;
    return;
}

int cpp_plugin_enable(void)
{
    if (x52pro)
    {
        x52pro->start();
        x52pro->pause(false);
    }
    return 1;
}

void cpp_plugin_disable(void)
{
    if (x52pro) x52pro->pause(true);
    return;
}

void cpp_plugin_recvmsg(int from, long msg, void* param)
{
    return;
}
