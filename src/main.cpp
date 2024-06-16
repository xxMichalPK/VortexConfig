// #define CFV_BUFFER_ONLY 1
// #define CFV_EMBEDDED_FUNCTIONS 1
#include "VortexConfig.h"

// Here we test the library
int main() {
	if (!cfv_open("Sample.cfv")) {
		printf("Failed to open file!\n\r");
	}

	const char* keyValue = cfv_get_string("nested", "object");
	printf("For key \"object\" the value is: \"%s\"\n\r", keyValue);

	const TCFVKey* parent = cfv_get_node("nested", "object");
	const char* nestedValue = cfv_get_string_from_node(parent, "an");
	printf("For nested key \"this\" in object \"object\" the value is: \"%s\"\n\r", nestedValue);

	const TCFVKey* nestedParent = cfv_get_node_from_node(parent, "an");
	const char* deeplyNestedValue = cfv_get_string_from_node(nestedParent, "object");
	printf("For deeply nested key \"object\" in object \"an\" in object \"object\" the value is: \"%s\"\n\r", deeplyNestedValue);

	cfv_clear();
	return 0;
}