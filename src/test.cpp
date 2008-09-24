#include <stdio.h>
#include "x52data.h"

int main(int argc, char* argv[])
{
	x52datasource_t* source = new x52datasource_t("hello world");
	const char* test = *source;
	delete source;
	printf("data: %s\n", test);
	test = 0;
	return 0;
}

