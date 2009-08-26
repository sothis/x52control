#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern int XPluginEnable(void);
extern void XPluginDisable(void);

int main(int argc, char* argv[], char* envp[])
{
	XPluginDisable();
	XPluginEnable();
	getchar();
	XPluginDisable();
	XPluginEnable();
	XPluginDisable();
	return 0;
}

