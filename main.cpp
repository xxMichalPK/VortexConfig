#include "VortexConfig.h"

// Here we test the library
int main() {
	if (!cfv_open("Sample.cfv")) {
		printf("Failed to open file!\n\r");
	}

	const char* keyValue = cfv_get_string("section", "key");
	printf("For key \"key\" the value is: \"%s\"\n\r", keyValue);

	cfv_clear();
	return 0;
}