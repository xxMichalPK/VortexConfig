# VortexConfig

VortexConfig (VortexConf) is a custom configuration parser designed to be simple, cross-platform, and efficient.
It is suitable for use in both standard applications and embedded systems with minimal resource requirements.
This parser supports a unique configuration format and provides an easy-to-use interface for reading configuration files.

### Features

- **Cross-Platform**: Works on Windows, macOS, Linux, and embedded systems.
- **Minimal Requirements**: Only requires `malloc`, `calloc`, `realloc`, and `free` from the C standard library, optionally `fopen`, `fclose`, `freak`, `fseek` and `ftell` for reading configuration files.
- **Custom Syntax**: Supports a unique, intuitive configuration syntax with sections, key-value pairs, arrays, and objects.
- **Comment Support**: Inline and block comments for better readability and documentation of configuration files.

### Getting Started

#### Prerequisites

- C compiler (GCC, Clang, MSVC, etc.)
- Make sure your project supports dynamic memory allocation (`malloc`, `calloc`, `realloc`, `free`).

#### Usage

1. **Include the Parser Code**

   Copy the `VortexConfig.h` header into your project

2. **Include the Header File**

	```c
		#include "VortexConfig.h"
	```

3. **Initialize the Parser**

	```c
		cfv_open("Sample.cfv");
	```

	or

	```c
		cfv_set_buffer([your_buffer], [size_of_the_buffer_]);
		cfv_parse();
	```

4. **Access Parsed Data**

### License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

### Acknowledgments
Inspired by various configuration file formats like INI and TOML. First appeared in VortexOS - a standalone hobby operating system