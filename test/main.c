#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern int XPluginEnable(void);
extern void XPluginDisable(void);

int main(int argc, char* argv[], char* envp[])
{
#if 0
	XPluginDisable();
#endif
	XPluginEnable();
	getchar();
	XPluginDisable();
#if 0
	XPluginEnable();
	XPluginDisable();
#endif
	return 0;
}

