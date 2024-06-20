# VortexConfig

VortexConfig (VortexConf) is a custom configuration parser designed to be simple, cross-platform, and efficient.
It is suitable for use in both standard applications and embedded systems with minimal resource requirements.
This parser supports a unique configuration format and provides an easy-to-use interface for reading configuration files.

### Features

- **Cross-Platform**: Works on Windows, macOS, Linux, and embedded systems.
- **Minimal Requirements**: Only requires `malloc`, `calloc`, `realloc`, and `free` from the C standard library, optionally `fopen`, `fclose`, `fread`, `fseek` and `ftell` for reading configuration files.
- **Custom Syntax**: Supports a unique, intuitive configuration syntax with sections, key-value pairs, arrays, and objects.
- **Comment Support**: Inline and block comments for better readability and documentation of configuration files.

### Getting Started

#### Prerequisites

- C compiler (GCC, Clang, MSVC, etc.)
- Make sure your project supports dynamic memory allocation (`malloc`, `calloc`, `realloc`, `free`).

#### Usage

1. **Include the Parser Code**

   Copy the `vcfg` directory from the `include` directory into your project include directory

2. **Include the Header File**

	```c
	#include "vcfg/VortexConfig.h"
	```

3. **Initialize the Parser**

	**C**

	```c
	VCFG_Parser parserObject = { 0 };
	vcfg_open(&parserObject, "File.vcfg");
	```

	or

	```c
	VCFG_Parser parserObject = { 0 };
	const char myBuffer[512] = { .... };
	vcfg_set_buffer(&parserObject, myBuffer, sizeof(myBuffer));
	vcfg_parse(&parserObject);
	```

	**C++**

	```cpp
	VCFG_Parser parserObject;
	parserObject.Open("File");
	```

	or

	```cpp
	VCFG_Parser parserObject;
	const char myBuffer[512] = { .... };
	parserObject.SetBuffer(myBuffer, sizeof(myBuffer));
	parserObject.Parse();
	```

4. **Access Parsed Data**

	**C**	

	```c
	const char* value = vcfg_get_string(&parserObject, "section", "key");
	int intVal = vcfg_get_int(&parserObject, "section", "intKey");
	float floatVal = vcfg_get_float(&parserObject, "section", "floatKey");
	bool boolVal = vcfg_get_bool(&parserObject, "section", "boolKey");

	// Nested keys
	const CFV_Node* node = vcfg_get_node(&parserObject, "section", "key");
	const char* nestedValue = vcfg_get_string_from_node(&parserObject, node, "key");
	// ...
	```

	**C++**

	```cpp
	const char* value = parserObject.GetString("section", "key");
	int intVal = parserObject.GetInt("section", "intKey");
	float floatVal = parserObject.GetFloat("section", "floatKey");
	bool boolVal = parserObject.GetBool("section", "boolKey");

	// Nested keys
	const CFV_Node* node = parserObject.GetNode("section", "key");
	const char* nestedValue = parserObject.GetNode(node, "key");
	// ...
	```

### License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

### Acknowledgments
Inspired by various configuration file formats like INI and TOML. First appeared in VortexOS - a standalone hobby operating system