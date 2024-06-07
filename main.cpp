#include "VortexConfig.h"

// Here we test the library
int main() {
	if (!cfv_open("Sample.cfv")) {
		printf("Failed to open file!\n\r");
	}

	cfv_clear();
	return 0;
}