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

	const CFV_Node* parent = cfv_get_node("nested", "object");
	const char* nestedValue = cfv_get_string_from_node(parent, "an");
	printf("For nested key \"an\" in object \"object\" the value is: \"%s\"\n\r", nestedValue);

	const CFV_Node* nestedParent = cfv_get_node_from_node(parent, "an");
	const char* deeplyNestedValue = cfv_get_string_from_node(nestedParent, "object");
	printf("For deeply nested key \"object\" in object \"an\" in object \"object\" the value is: \"%s\"\n\r", deeplyNestedValue);

	const CFV_Node* arrParent = cfv_get_node("nested", "array");
	const char* arrValue = cfv_get_string_from_node(arrParent, "3");
	printf("For nested key at index 0 in array \"array\" the value is: \"%s\"\n\r", arrValue);

	cfv_clear();
	return 0;
}