// #define VCFG_BUFFER_ONLY 1
// #define VCFG_EMBEDDED_FUNCTIONS 1
#include "VortexConfig.h"
#include <iostream>

// Here we test the library
int main() {
	if (!vcfg_open("Sample.vcfg")) {
		printf("Failed to open the specified file!\n\r");
		return 1;
	}

	std::cout << "Testing section separation:\n\r";

	const char* rootIsRoot = vcfg_get_string(nullptr, "is_root_section");
	const char* notRootIsRoot = vcfg_get_string("first_section", "is_root_section");
	std::cout << "The value of \"is_root_section\" in the root section is: " << rootIsRoot << "\n\r";
	std::cout << "The value of \"is_root_section\" in other section is: " << notRootIsRoot << "\n\r";


	std::cout << "\n\rTesting keys in quotes:\n\r";
	const char* keyInQuotesValue = vcfg_get_string("first_section", "should this keytype stay?");
	std::cout << "The answer to \"should this keytype stay?\" is: " << keyInQuotesValue << "\n\r";

	std::cout << "\n\rTesting section names with spaces:\n\r";
	const char* spaceSectionValue = vcfg_get_string("space section", "does_this_work");
	std::cout << "The answer to \"does_this_work\" regarding spaces in section names is: " << spaceSectionValue << "\n\r";

	std::cout << "\n\rTesting different value types:\n\r";
	int intVal = vcfg_get_int("first_section", "lines_of_code");
	double floatVal = vcfg_get_float("first_section", "satisfaction_level");
	bool boolVal = vcfg_get_bool(nullptr, "is_root_section");
	std::cout << "Integer value \"lines_of_code\" is: " << intVal << "\n\r";
	std::cout << "Floating point value \"satisfaction_level\" is: " << floatVal << "\n\r";
	std::cout << "Boolean value \"is_root_section\" is: " << boolVal << "\n\r";

	std::cout << "\n\rTesting nested keys (objects and arrays):\n\r";
	const char* objectValue = vcfg_get_string("nested_keys", "an_object");
	const VCFG_Node* objectNode = vcfg_get_node("nested_keys", "an_object");
	const char* insideObjectValue1 = vcfg_get_string_from_node(objectNode, "inner_key_1");
	const char* insideObjectArrayValue = vcfg_get_string_from_node(objectNode, "inner_array");
	const VCFG_Node* insideObjectArray = vcfg_get_node_from_node(objectNode, "inner_array");
	int doubleNestedValue = vcfg_get_int_from_node(insideObjectArray, "0");
	std::cout << "The value of the \"an_object\" object is: " << objectValue << "\n\r";
	std::cout << "The value of the \"inner_key_1\" inside the object is: " << insideObjectValue1 << "\n\r";
	std::cout << "The value of the \"inner_array\" inside the object is: " << insideObjectArrayValue << "\n\r";
	std::cout << "The value of the first element in \"inner_array\" inside the object is: " << doubleNestedValue << "\n\r";

	vcfg_clear();
	return 0;
}