/*
 * SimpleConfigLib.h - A simple library for reading and generating configuration files
 *
 * MIT License
 * 
 * Copyright (c) 2024 Michał Pazurek
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#pragma once
// Include guards just in case
#ifndef SIMPLE_CONFIG_LIB
#define SIMPLE_CONFIG_LIB 1

// Simplify os detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define OS_WINDOWS 1
#elif defined(__linux__)
	#define OS_LINUX 1
#endif

// Helper macros
#ifndef CFV_IS_WHITESPACE
	#define CFV_IS_WHITESPACE(ch) ((ch == ' ') || ((ch >= '\b') && (ch <= '\r')))
#endif

// All the necessary C code
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	#include <stdint.h>
	// We need stdlib.h for malloc, calloc, realloc and free.
	// Those can be replaced by a custom implementation in an embedded system
	#include <stdlib.h>

	typedef struct SCFVKey {
		char* name;
		char* value;
		uint32_t childCount;
		struct SCFVKey* children;
	} TCFVKey;

	typedef struct SCFVSection {
		char* name;
		uint32_t keyCount;
		TCFVKey* keys;
	} TCFVSection;

	// The raw data of the configuration (file)
	static const char* m_configBuffer = 0;
	static size_t m_configBufferLength = 0;

	// The parsed data
	static TCFVSection* m_parsedData = 0;
	static uint32_t m_sectionCount = 0;

	// Compatibility functions for embedded systems
	#if (!defined(OS_WINDOWS) && !defined(OS_LINUX)) || defined(CFV_EMBEDDED_FUNCTIONS)
		inline void cfvinternal_memcpy(void* dst, const void* src, size_t count) {
			while (count--) {
				*((char*)dst) = *((char*)src);
			}
		}
	#else
		#include <string.h>

		inline void cfvinternal_memcpy(void* dst, const void* src, size_t count) {
			memcpy(dst, src, count);
		}
	#endif

	/**
	 *	@brief Set config buffer.
	 *	
	 *	Sets the raw data buffer to the specified input buffer
	 * 
	 *	@param inputBuffer - the desired raw data buffer
	 *	@param dataLength - length of the input
	 */
	inline void cfv_set_buffer(const char* inputBuffer, size_t dataLength) {
		// If the buffer was already set try to free it
		if (m_configBuffer) {
			free((void*)m_configBuffer);
			m_configBuffer = 0;
		}

		m_configBuffer = inputBuffer;
		m_configBufferLength = dataLength;
	}


	inline size_t cfvinternal_skipwhitespace(const char **dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		while (CFV_IS_WHITESPACE(*internalDataPtr) && (internalDataPtr < dataEndPtr)) {
			++internalDataPtr;
		}

		if (internalDataPtr == *dataPtr) return 0;
		
		size_t skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}


	inline size_t cfvinternal_skiplinecomment(const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		// Check if we have a comment "//"
		if ((*internalDataPtr != '/') || (*(internalDataPtr + 1) != '/')) return 0;

		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		while ((internalDataPtr < dataEndPtr) && (*internalDataPtr != '\n')) {
			++internalDataPtr;
		}

		if (*internalDataPtr == '\n') ++internalDataPtr;
		
		if (internalDataPtr == *dataPtr) return 0;

		size_t skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}
	inline size_t cfvinternal_skipblockcomment(const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		// Check if we have a comment block "/*"
		if ((*internalDataPtr != '/') || (*(internalDataPtr + 1) != '*')) return 0;
		
		// Skip the first comment block characters to avoid something like this: "/*/"
		internalDataPtr += 2;

		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		while ((internalDataPtr < dataEndPtr)) {
			if ((*internalDataPtr == '*') && (*(internalDataPtr + 1) == '/')) {
				internalDataPtr += 2;
				break;
			}
			++internalDataPtr;
		}

		if (internalDataPtr == *dataPtr) return 0;

		size_t skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	inline size_t cfvinternal_skipcomments(const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		size_t skippedCount = 0;
		skippedCount += cfvinternal_skiplinecomment(&internalDataPtr);
		skippedCount += cfvinternal_skipblockcomment(&internalDataPtr);

		if ((internalDataPtr == *dataPtr) || (skippedCount == 0)) return 0;

		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	inline size_t cfvinternal_createsection(const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		if (*internalDataPtr != '[') return 0;

		// If we have a [ it means we have to create a new section that we "push" on top
		// of the previously added section (for that reason section names have to be unique)
		++internalDataPtr;
		size_t nameLength = 0;
		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		while ((*internalDataPtr != ']') && (internalDataPtr < dataEndPtr)) {
			++internalDataPtr;
			++nameLength;
		}
		if (*internalDataPtr == ']') ++internalDataPtr;
		size_t skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;

		// We don't allow empty sections -> []
		if (nameLength <= 0) return skippedCount;

		++m_sectionCount;
		TCFVSection* newSections = (TCFVSection*)realloc(m_parsedData, m_sectionCount * sizeof(TCFVSection));
		if (!newSections) {
			--m_sectionCount;
			return skippedCount;
		}

		newSections[m_sectionCount - 1].keyCount = 0;
		newSections[m_sectionCount - 1].keys = 0;
		newSections[m_sectionCount - 1].name = (char*)malloc(nameLength + 1);
		if (!(newSections[m_sectionCount - 1].name)) {
			--m_sectionCount;
			return skippedCount;
		}

		cfvinternal_memcpy((void*)(newSections[m_sectionCount - 1].name), (void*)(internalDataPtr - nameLength - 1), nameLength);
		newSections[m_sectionCount - 1].name[nameLength] = '\0';

		m_parsedData = newSections;

		return skippedCount;
	}

	inline size_t cfvinternal_parsearray(const char** dataPtr) {
		return 0;
	}
	inline size_t cfvinternal_parseobject(const char** dataPtr) {
		return 0;
	}

	inline size_t cfvinternal_parsekeyvalue(const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		// TODO:
		//	- actually parse the key-value pairs

		if (internalDataPtr == *dataPtr) return 0;

		size_t skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	/**
	 *	@brief Parse configuration.
	 * 
	 *	Parses the configuration data stored in the raw data buffer.
	 *	This function has to be called explicitly when using static buffers.
	 *	When using cfv_open, this function is called automatically
	 * 
	 *	@returns 0 - Failure, 1 - Success
	 */
	inline int cfv_parse() {
		if (!m_configBuffer || !m_configBufferLength) return 0;

		// We need to store the end of the buffer to avoid overflowing
		const char* dataEndPtr = m_configBuffer + m_configBufferLength;

		// We work on a temporary variable to ensure the original pointer is in tact
		const char* internalDataPtr = m_configBuffer;

		// Allocate buffer for the root section
		m_parsedData = (TCFVSection*)calloc(1, sizeof(TCFVSection));
		if (!m_parsedData) return 0;
		m_sectionCount = 1;

		// TODO:
		//	- Maybe a better error handling?
		//	- Test for edge cases
		while (internalDataPtr < dataEndPtr) {
			// Skip all whitespaces and comments
			if (cfvinternal_skipwhitespace(&internalDataPtr)) continue;
			if (cfvinternal_skipcomments(&internalDataPtr)) continue;

			// If there is a section -> [sectionname] -> create a new section
			// in the parsed data structure and push it to the end
			if (cfvinternal_createsection(&internalDataPtr)) continue;

			// The only thing left to do is to parse the key-value pairs
			if (cfvinternal_parsekeyvalue(&internalDataPtr)) continue;
			
			// If everything returned with 0 (no data was parsed) just break
			break;
		}

		return 1;
	}

	// For embedded systems we can avoid using file operations and use only statically defined buffers
	// so in case someone doesn't have the file operations implemented we use preprocessor definitions
	#if !defined(CFV_BUFFER_ONLY)
	#include <stdio.h>

	static FILE* m_currentConfigFile = 0;

	/**
	 *	@brief Open configuration file.
	 * 
	 *	Opens/Creates the configuration file and reads the content to the internal buffer
	 * 
	 *	@param s_path - path to the configuration file
	 * 
	 *	@returns 0 - Failure, 1 - Success
	 */
	inline int cfv_open(const char *s_path) {
		// If a file was already opened close the previous one to avoid memory leaks
		if (m_currentConfigFile) {
			fclose(m_currentConfigFile);
			m_currentConfigFile = 0;
		}
		
		// Open the desired file
		m_currentConfigFile = fopen(s_path, "a+b");
		if (!m_currentConfigFile) {
			perror("Failed to open the specified file\n\r");
			return 0; // Failure
		}

		// Create a buffer for the file to read to
		fseek(m_currentConfigFile, 0, SEEK_END);
		size_t fileSize = ftell(m_currentConfigFile);
		fseek(m_currentConfigFile, 0, SEEK_SET);

		// The file was created so just exit with success
		if (fileSize == 0) return 1;

		// Clear the previously used buffer
		if (m_configBuffer) {
			free((void*)m_configBuffer);
			m_configBuffer = 0;
		}

		// Allocate memory
		m_configBuffer = (char*)malloc((fileSize + 1) * sizeof(char));
		if (!m_configBuffer) {
			perror("Failed to allocate memory for the configuration file\n\r");
			return 0;
		}

		// Read the file contents into the buffer
		if (fread((void*)m_configBuffer, sizeof(char), fileSize, m_currentConfigFile) != fileSize) {
			perror("Failed to read the entire configuration file\n\r");
			return 0;
		};

		m_configBufferLength = fileSize;
		
		return cfv_parse();
	}
	#endif // CFV_BUFFER_ONLY

	inline void cfvinternal_clear_key(TCFVKey key) {
		free((void*)(key.name));
		key.name = 0;

		if (key.value) {
			free((void*)(key.value));
			key.value = 0;
		}

		if (key.childCount) {
			for (uint32_t i = 0; i < key.childCount; i++) {
				cfvinternal_clear_key(key.children[i]);
			}
			free((void*)(key.children));
			key.children = 0;
			
			key.childCount = 0;
		}
	}

	inline void cfvinternal_clear_section(TCFVSection section) {
		if (section.name) {
			free((void*)(section.name));
			section.name = 0;
		}

		if (section.keyCount) {
			for (uint32_t i = 0; i < section.keyCount; i++) {
				cfvinternal_clear_key(section.keys[i]);
			}
			free(section.keys);
			section.keys = 0;
			section.keyCount = 0;
		}
	}

	/**
	 *	@brief Clear the library.
	 *
	 *	Frees all the allocated buffers and closes the configuration files.
	 *	It has to be run after we end working with the library to avoid memory leaks
	 */
	inline void cfv_clear() {
		if (m_configBuffer) {
			free((void*)m_configBuffer);
			m_configBuffer = 0;
		}

		if (m_sectionCount) {
			for (uint32_t i = 0; i < m_sectionCount; i++) {
				cfvinternal_clear_section(m_parsedData[i]);
			}

			free((void*)(m_parsedData));
			m_parsedData = 0;

			m_sectionCount = 0;
		}

		#if !defined(CFV_BUFFER_ONLY)
			if (m_currentConfigFile) {
				fclose(m_currentConfigFile);
				m_currentConfigFile = 0;
			}
		#endif // CFV_BUFFER_ONLY
	}

#ifdef __cplusplus
}
#endif // __cplusplus

// C++ wrapper
#ifdef __cplusplus
	
#endif // __cplusplus

#endif // SIMPLE_CONFIG_LIB