/*
 * VortexConfig.h - A simple library for reading and generating configuration files
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
#ifndef VORTEX_CONFIG_LIB
#define VORTEX_CONFIG_LIB 1

// Simplify os detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define OS_WINDOWS 1
#elif defined(__linux__)
	#define OS_LINUX 1
#endif

// Helper macros
#ifndef VCFG_IS_WHITESPACE
	#define VCFG_IS_WHITESPACE(ch) ((ch == ' ') || ((ch >= '\b') && (ch <= '\r')))
#endif

#ifndef VCFG_IS_NUMBER
	#define VCFG_IS_NUMBER(ch) ((ch >= '0') && (ch <= '9'))
#endif

// All the necessary C code
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	#include <stdint.h>
	// We need stdlib.h for malloc, calloc, realloc and free.
	// Those can be replaced by a custom implementation in an embedded system
	#include <stdlib.h>

	typedef struct SVCFGKey {
		char* name;
		char* value;
		uint32_t childCount;
		struct SVCFGKey* children;
	} TVCFGKey;
	typedef TVCFGKey VCFG_Node;

	typedef struct SVCFGSection {
		char* name;
		uint32_t keyCount;
		TVCFGKey* keys;
	} TVCFGSection;

	// The raw data of the configuration (file)
	static const char* m_configBuffer = 0;
	static size_t m_configBufferLength = 0;

	// The parsed data
	static TVCFGSection* m_parsedData = 0;
	static uint32_t m_sectionCount = 0;

	// Compatibility functions for embedded systems
	#if (!defined(OS_WINDOWS) && !defined(OS_LINUX)) || defined(VCFG_EMBEDDED_FUNCTIONS)
		/**
		 *	@brief C library memcpy implementation
		 */
		inline void vcfginternal_memcpy(void* dst, const void* src, size_t count) {
			unsigned char* srcPtr = (unsigned char*)src;
			unsigned char* dstPtr = (unsigned char*)dst;

			while (count--) {
				*(dstPtr++) = *(srcPtr++);
			}
		}

		/**
		 *	@brief C library strlen implementation
		 */
		inline size_t vcfginternal_strlen(const char* str) {
			size_t len = 0;
			while (*(str++) != '\0') ++len;
			return len;
		}

		/**
		 *	@brief C library strcmp implementation
		 */
		inline int vcfginternal_strcmp(const char* str1, const char* str2) {
			if (str1 == str2) return 0;
			if (str1 == 0 || str2 == 0) return -1;
			
			while (*str1 == *str2) {
				if (*str1 == '\0' || *str2 == '\0') {
					return 0;
				}
				++str1;
				++str2;
			}

			return (*str1 - *str2);
		}
	#else
		#include <string.h>

		/**
		 *	@brief C library memcpy implementation
		 */
		inline void vcfginternal_memcpy(void* dst, const void* src, size_t count) {
			memcpy(dst, src, count);
		}

		/**
		 *	@brief C library strlen implementation
		 */
		inline size_t vcfginternal_strlen(const char* str) {
			return strlen(str);
		}

		/**
		 *	@brief C library strcmp implementation
		 */
		inline int vcfginternal_strcmp(const char* str1, const char* str2) {
			if (str1 == str2) return 0;
			if (str1 == 0 || str2 == 0) return -999999;
			return strcmp(str1, str2);
		}
	#endif

	/**
	 *	@brief String to integer conversion.
	 * 
	 *	@param str - the stringified number
	 * 
	 *	@returns (int64_t) the number as an integer
	 */
	inline int64_t vcfginternal_strtoint(const char* str) {
		if (!str) return -1;

		int negative = 0;
		int64_t result = 0;

		if (*str == '-') {
			negative = 1;
			++str;
		}

		// If the first character is not a number there is no point in running the loop
		if (!VCFG_IS_NUMBER(*str)) return -1;

		while (*str != '\0') {
			if (!VCFG_IS_NUMBER(*str)) break;

			result *= 10;
			result += (*str) - '0';
			++str;
		}

		return (negative ? -result : result);
	}

	/**
	 *	@brief String to floating point number conversion.
	 *
	 *	@param str - the stringified number
	 *
	 *	@returns (double) the number as a floating point number
	 */
	inline double vcfginternal_strtofloat(const char* str) {
		if (!str) return -1;

		int negative = 0;
		double result = 0;

		if (*str == '-') {
			negative = 1;
			++str;
		}

		// If the first character is not a number, nor a dot, there is no point in running the loop
		if (!VCFG_IS_NUMBER(*str) && *str != '.') return -1;

		int isDecimalPart = 0;
		int decimalPlaces = 0;
		while (*str != '\0') {
			if (!VCFG_IS_NUMBER(*str) && *str != '.') break;
			if (isDecimalPart && *str == '.') break;	// In case there is a second dot
			if (*str == '.') {
				isDecimalPart = 1;
				++str;
				continue;
			}
			if (isDecimalPart) ++decimalPlaces;

			result *= 10;
			result += (*str) - '0';
			++str;
		}

		while (isDecimalPart && decimalPlaces) {
			result /= 10;
			--decimalPlaces;
		}

		return (negative ? -result : result);
	}

	/**
	 *	@brief Unsigned number to string conversion.
	 * 
	 *	WARNING: this function works only on positive numbers that fit in size_t
	 *	32bit numbers for 32bit machines and 64bit numbers for 64bit machines!
	 *
	 *	@param number - the number to convert
	 *
	 *	@returns (char*) the number as a string (null terminated)
	 */
	inline char* vcfginternal_unsignednumtostr(size_t number) {
		char* result = (char*)malloc(22 * sizeof(char));
		if (!result) return 0;

		if (number == 0) {
			result[0] = '0';
			result[1] = '\0';
			return result;
		}

		int pos = 0;
		while (number > 0) {
			result[pos++] = (number % 10) + '0';
			number /= 10;
		}

		result[pos--] = '\0';

		for (int i = 0; i < pos; i++, pos--) {
			char tmp = result[pos];
			result[pos] = result[i];
			result[i] = tmp;
		}

		return result;
	}

	/**
	 *	@brief Set config buffer.
	 *	
	 *	Sets the raw data buffer to the specified input buffer
	 * 
	 *	@param inputBuffer - the desired raw data buffer
	 *	@param dataLength - length of the input
	 */
	inline void vcfg_set_buffer(const char* inputBuffer, size_t dataLength) {
		// If the buffer was already set try to free it
		if (m_configBuffer) {
			free((void*)m_configBuffer);
			m_configBuffer = 0;
		}

		m_configBuffer = inputBuffer;
		m_configBufferLength = dataLength;
	}

	/**
	 *	@brief Skip whitespace.
	 * 
	 *	Skips all whitespace characters in a buffer
	 * 
	 *	@param dataPtr - pointer to the raw data buffer
	 * 
	 *	@returns (size_t) number of bytes skipped
	 */
	inline size_t vcfginternal_skipwhitespace(const char **dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		while (VCFG_IS_WHITESPACE(*internalDataPtr) && (internalDataPtr < dataEndPtr)) {
			++internalDataPtr;
		}

		if (internalDataPtr == *dataPtr) return 0;
		
		size_t skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	/**
	 *	@brief Skip line comment.
	 *
	 *	Skips an entire line starting at the comment sign -> //
	 *
	 *	@param dataPtr - pointer to the raw data buffer
	 *
	 *	@returns (size_t) number of bytes skipped
	 */
	inline size_t vcfginternal_skiplinecomment(const char** dataPtr) {
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

	/**
	 *	@brief Skip block comment.
	 *
	 *	Skips an entire commented block starting at the comment sign -> /* and ending with "* /"
	 *
	 *	@param dataPtr - pointer to the raw data buffer
	 *
	 *	@returns (size_t) number of bytes skipped
	 */
	inline size_t vcfginternal_skipblockcomment(const char** dataPtr) {
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

	/**
	 *	@brief Skip all comments.
	 *
	 *	Skips all types of comments
	 *
	 *	@param dataPtr - pointer to the raw data buffer
	 *
	 *	@returns (size_t) number of bytes skipped
	 */
	inline size_t vcfginternal_skipcomments(const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		size_t skippedCount = 0;
		skippedCount += vcfginternal_skiplinecomment(&internalDataPtr);
		skippedCount += vcfginternal_skipblockcomment(&internalDataPtr);

		if ((internalDataPtr == *dataPtr) || (skippedCount == 0)) return 0;

		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	/**
	 *	@brief Create a new section in the parsed data structure.
	 *
	 *	Parses the name of a section in the configuration file and creates a new section
	 *	structure with the appropriate name, then pushes the section to the end of the parsed data
	 *
	 *	@param dataPtr - pointer to the raw data buffer
	 *
	 *	@returns (size_t) number of bytes skipped
	 */
	inline size_t vcfginternal_createsection(const char** dataPtr) {
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
		if (nameLength == 0) return skippedCount;

		++m_sectionCount;
		TVCFGSection* newSections = (TVCFGSection*)realloc(m_parsedData, m_sectionCount * sizeof(TVCFGSection));
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

		vcfginternal_memcpy((void*)(newSections[m_sectionCount - 1].name), (void*)(internalDataPtr - nameLength - 1), nameLength);
		newSections[m_sectionCount - 1].name[nameLength] = '\0';

		m_parsedData = newSections;

		return skippedCount;
	}

	// Forward declare the needed function
	inline size_t vcfginternal_parsearray_keyvalue(const char** dataPtr, size_t valueIndex, TVCFGKey* keyValuePair);
	inline size_t vcfginternal_parsearray(const char** dataPtr, TVCFGKey* keyValuePair) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		if (*internalDataPtr != '[') return 0;
		++internalDataPtr;

		keyValuePair->value = (char*)malloc(8 * sizeof(char));
		if (keyValuePair->value) {
			vcfginternal_memcpy((void*)(keyValuePair->value), (void*)"[array]\0", 8);
		}
		
		vcfginternal_skipwhitespace(&internalDataPtr);

		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		size_t arrayIndex = 0;
		while ((internalDataPtr < dataEndPtr) && (*internalDataPtr != ']')) {
			// Skip all whitespaces and comments
			if (vcfginternal_skipwhitespace(&internalDataPtr)) continue;
			if (vcfginternal_skipcomments(&internalDataPtr)) continue;

			if (vcfginternal_parsearray_keyvalue(&internalDataPtr, arrayIndex, keyValuePair)) {
				vcfginternal_skipwhitespace(&internalDataPtr);
				if (*internalDataPtr == ',') {
					++internalDataPtr;
					++arrayIndex;
					continue;
				}

				// If there was no comma skip to the end of the array
				while ((internalDataPtr < dataEndPtr) && (*internalDataPtr != ']')) ++internalDataPtr;
			}
		}

		size_t skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	// Forward declare the needed function
	inline size_t vcfginternal_parseobject_keyvalue(const char** dataPtr, TVCFGKey* keyValuePair);
	inline size_t vcfginternal_parseobject(const char** dataPtr, TVCFGKey* keyValuePair) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		if (*internalDataPtr != '{') return 0;
		++internalDataPtr;

		keyValuePair->value = (char*)malloc(9 * sizeof(char));
		if (keyValuePair->value) {
			vcfginternal_memcpy((void*)(keyValuePair->value), (void*)"{object}\0", 9);
		}

		vcfginternal_skipwhitespace(&internalDataPtr);

		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		while ((internalDataPtr < dataEndPtr) && (*internalDataPtr != '}')) {
			// Skip all whitespaces and comments
			if (vcfginternal_skipwhitespace(&internalDataPtr)) continue;
			if (vcfginternal_skipcomments(&internalDataPtr)) continue;

			if (vcfginternal_parseobject_keyvalue(&internalDataPtr, keyValuePair)) {
				vcfginternal_skipwhitespace(&internalDataPtr);
				if (*internalDataPtr == ',') {
					++internalDataPtr;
					continue;
				}
				// If there was no comma skip to the end of the object
				while ((internalDataPtr < dataEndPtr) && (*internalDataPtr != '}')) ++internalDataPtr;
			}
		}

		size_t skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}
	inline size_t vcfginternal_parsevalue(const char** dataPtr, TVCFGKey *keyValuePair) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		int valueInQuotes = (*internalDataPtr == '"') ? 1 : 0;
		if (valueInQuotes) ++internalDataPtr;

		size_t valueLength = 0;
		const char* valueStart = internalDataPtr;
		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		while ((internalDataPtr < dataEndPtr)) {
			if (valueInQuotes && (*internalDataPtr == '"')) {
				++internalDataPtr;
				break;
			}
			if (!valueInQuotes && (VCFG_IS_WHITESPACE(*internalDataPtr) || (*internalDataPtr == ',') || (*internalDataPtr == ';'))) break;

			++internalDataPtr;
			++valueLength;
		}

		size_t skippedCount = internalDataPtr - *dataPtr;
		if (valueLength == 0) {
			*dataPtr = internalDataPtr;
			return skippedCount;
		}

		// Allocate memory for the value
		keyValuePair->value = (char*)malloc(valueLength + 1);
		if (!(keyValuePair->value)) {
			keyValuePair->value = 0;
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		vcfginternal_memcpy((void*)(keyValuePair->value), (void*)valueStart, valueLength);
		keyValuePair->value[valueLength] = '\0';

		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	inline size_t vcfginternal_parsearray_keyvalue(const char** dataPtr, size_t valueIndex, TVCFGKey* keyValuePair) {
		if (!dataPtr || !keyValuePair) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		char* keyName = vcfginternal_unsignednumtostr(valueIndex);
		if (!keyName) return 0;

		vcfginternal_skipwhitespace(&internalDataPtr);

		// Don't allow empty keys
		size_t skippedCount = internalDataPtr - *dataPtr;

		// Add the key to the parent key
		keyValuePair->childCount++;
		TVCFGKey* newChildren = (TVCFGKey*)realloc((void*)(keyValuePair->children), keyValuePair->childCount * sizeof(TVCFGKey));
		if (!newChildren) {
			keyValuePair->childCount--;
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		newChildren[keyValuePair->childCount - 1].childCount = 0;
		newChildren[keyValuePair->childCount - 1].children = 0;
		newChildren[keyValuePair->childCount - 1].value = 0;
		newChildren[keyValuePair->childCount - 1].name = keyName;

		keyValuePair->children = newChildren;

		vcfginternal_skipwhitespace(&internalDataPtr);

		// Check if the value is an array or object
		if (*internalDataPtr == '{') {
			vcfginternal_parseobject(&internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}
		else if (*internalDataPtr == '[') {
			vcfginternal_parsearray(&internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}
		else {
			vcfginternal_parsevalue(&internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}

		skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	inline size_t vcfginternal_parseobject_keyvalue(const char** dataPtr, TVCFGKey* keyValuePair) {
		if (!dataPtr || !keyValuePair) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		int keyInQuotes = (*internalDataPtr == '"') ? 1 : 0;
		if (keyInQuotes) ++internalDataPtr;

		size_t keyLength = 0;
		const char* keyStart = internalDataPtr;
		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		while ((internalDataPtr < dataEndPtr)) {
			if (keyInQuotes && (*internalDataPtr == '"')) {
				++internalDataPtr;
				break;
			}
			if (!keyInQuotes && (VCFG_IS_WHITESPACE(*internalDataPtr) || (*internalDataPtr == '='))) break;

			++internalDataPtr;
			++keyLength;
		}

		vcfginternal_skipwhitespace(&internalDataPtr);

		// Don't allow empty keys
		size_t skippedCount = internalDataPtr - *dataPtr;
		if (keyLength == 0) {
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		// If there's no = it's not a valid key-value pair
		if (*internalDataPtr != '=') {
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		// Add the key to the parent key
		keyValuePair->childCount++;
		TVCFGKey* newChildren = (TVCFGKey*)realloc((void*)(keyValuePair->children), keyValuePair->childCount * sizeof(TVCFGKey));
		if (!newChildren) {
			keyValuePair->childCount--;
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		newChildren[keyValuePair->childCount - 1].childCount = 0;
		newChildren[keyValuePair->childCount - 1].children = 0;
		newChildren[keyValuePair->childCount - 1].value = 0;
		newChildren[keyValuePair->childCount - 1].name = (char*)malloc(keyLength + 1);
		if (!(newChildren[keyValuePair->childCount - 1].name)) {
			keyValuePair->childCount--;
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		vcfginternal_memcpy((void*)(newChildren[keyValuePair->childCount - 1].name), (void*)keyStart, keyLength);
		newChildren[keyValuePair->childCount - 1].name[keyLength] = '\0';

		keyValuePair->children = newChildren;

		// Skip the equal sign
		++internalDataPtr;

		vcfginternal_skipwhitespace(&internalDataPtr);

		// Check if the value is an array or object
		if (*internalDataPtr == '{') {
			vcfginternal_parseobject(&internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}
		else if (*internalDataPtr == '[') {
			vcfginternal_parsearray(&internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}
		else {
			vcfginternal_parsevalue(&internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}

		skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	inline size_t vcfginternal_parsekeyvalue(const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		// We support keys in quotes so we need to account for that.
		int keyInQuotes = (*internalDataPtr == '"') ? 1 : 0;
		if (keyInQuotes) ++internalDataPtr;

		size_t keyLength = 0;
		const char* keyStart = internalDataPtr;
		const char* dataEndPtr = m_configBuffer + m_configBufferLength;
		while ((internalDataPtr < dataEndPtr)) {
			if (keyInQuotes && (*internalDataPtr == '"')) { 
				++internalDataPtr;
				break;
			}
			if (!keyInQuotes && (VCFG_IS_WHITESPACE(*internalDataPtr) || (*internalDataPtr == '='))) break;

			++internalDataPtr;
			++keyLength;
		}

		vcfginternal_skipwhitespace(&internalDataPtr);

		// Don't allow empty keys
		size_t skippedCount = internalDataPtr - *dataPtr;
		if (keyLength == 0) {
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		// If there's no = it's not a valid key-value pair
		if (*internalDataPtr != '=') {
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		// Add the key to the current section
		m_parsedData[m_sectionCount - 1].keyCount++;
		TVCFGKey* newKeys = (TVCFGKey*)realloc((void*)(m_parsedData[m_sectionCount - 1].keys), m_parsedData[m_sectionCount - 1].keyCount * sizeof(TVCFGKey));
		if (!newKeys) {
			m_parsedData[m_sectionCount - 1].keyCount--;
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1].childCount = 0;
		newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1].children = 0;
		newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1].value = 0;
		newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1].name = (char*)malloc(keyLength + 1);
		if (!(newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1].name)) {
			m_parsedData[m_sectionCount - 1].keyCount--;
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		vcfginternal_memcpy((void*)(newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1].name), (void*)keyStart, keyLength);
		newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1].name[keyLength] = '\0';

		m_parsedData[m_sectionCount - 1].keys = newKeys;

		// Skip the equal sign
		++internalDataPtr;

		vcfginternal_skipwhitespace(&internalDataPtr);

		// Check if the value is an array or object
		if (*internalDataPtr == '{') {
			vcfginternal_parseobject(&internalDataPtr, &(newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1]));
		}
		else if (*internalDataPtr == '[') {
			vcfginternal_parsearray(&internalDataPtr, &(newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1]));
		}
		else {
			vcfginternal_parsevalue(&internalDataPtr, &(newKeys[m_parsedData[m_sectionCount - 1].keyCount - 1]));
		}

		skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	/**
	 *	@brief Parse configuration.
	 * 
	 *	Parses the configuration data stored in the raw data buffer.
	 *	This function has to be called explicitly when using static buffers.
	 *	When using vcfg_open, this function is called automatically
	 * 
	 *	@returns 0 - Failure, 1 - Success
	 */
	inline int vcfg_parse() {
		if (!m_configBuffer || !m_configBufferLength) return 0;

		// We need to store the end of the buffer to avoid overflowing
		const char* dataEndPtr = m_configBuffer + m_configBufferLength;

		// We work on a temporary variable to ensure the original pointer is in tact
		const char* internalDataPtr = m_configBuffer;

		// Allocate buffer for the root section
		m_parsedData = (TVCFGSection*)calloc(1, sizeof(TVCFGSection));
		if (!m_parsedData) return 0;
		m_sectionCount = 1;

		// TODO:
		//	- Maybe a better error handling?
		//	- Test for edge cases
		while (internalDataPtr < dataEndPtr) {
			// Skip all whitespaces and comments
			if (vcfginternal_skipwhitespace(&internalDataPtr)) continue;
			if (vcfginternal_skipcomments(&internalDataPtr)) continue;

			// If there is a section -> [sectionname] -> create a new section
			// in the parsed data structure and push it to the end
			if (vcfginternal_createsection(&internalDataPtr)) continue;

			// The only thing left to do is to parse the key-value pairs
			if (vcfginternal_parsekeyvalue(&internalDataPtr)) continue;
			
			// If everything returned with 0 (no data was parsed) just break
			break;
		}

		return 1;
	}

	// For embedded systems we can avoid using file operations and use only statically defined buffers
	// so in case someone doesn't have the file operations implemented we use preprocessor definitions
	#if !defined(VCFG_BUFFER_ONLY)
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
	inline int vcfg_open(const char *s_path) {
		// If a file was already opened close the previous one to avoid memory leaks
		if (m_currentConfigFile) {
			fclose(m_currentConfigFile);
			m_currentConfigFile = 0;
		}
		
		// Open the desired file
		errno_t fileOpenFailed = fopen_s(&m_currentConfigFile, s_path, "rb");
		if (fileOpenFailed || !m_currentConfigFile) {
			perror("FOPEN()");
			return 0; // Failure
		}

		// Create a buffer for the file to read to
		fseek(m_currentConfigFile, 0, SEEK_END);
		size_t fileSize = ftell(m_currentConfigFile);
		fseek(m_currentConfigFile, 0, SEEK_SET);

		// Clear the previously used buffer
		if (m_configBuffer) {
			free((void*)m_configBuffer);
			m_configBuffer = 0;
		}

		// Allocate memory
		m_configBuffer = (char*)malloc((fileSize + 1) * sizeof(char));
		if (!m_configBuffer) {
			perror("MALLOC()");
			return 0;
		}

		// Read the file contents into the buffer
		if (fread((void*)m_configBuffer, sizeof(char), fileSize, m_currentConfigFile) != fileSize) {
			perror("FREAD()");
			return 0;
		};

		m_configBufferLength = fileSize;
		
		return vcfg_parse();
	}
	#endif // VCFG_BUFFER_ONLY

	/**
	 *	@brief Clear key.
	 *
	 *	Clears the entire key by freeing the allocated memory and recursively calling
	 *	itself for all the nested child keys
	 */
	inline void vcfginternal_clear_key(TVCFGKey key) {
		free((void*)(key.name));
		key.name = 0;

		if (key.value) {
			free((void*)(key.value));
			key.value = 0;
		}

		if (key.childCount) {
			for (uint32_t i = 0; i < key.childCount; i++) {
				vcfginternal_clear_key(key.children[i]);
			}
			free((void*)(key.children));
			key.children = 0;
			
			key.childCount = 0;
		}
	}

	/**
	 *	@brief Clear section.
	 * 
	 *	Clears the entire section by clearing all the keys and freeing the allocated memory
	 */
	inline void vcfginternal_clear_section(TVCFGSection section) {
		if (section.name) {
			free((void*)(section.name));
			section.name = 0;
		}

		if (section.keyCount) {
			for (uint32_t i = 0; i < section.keyCount; i++) {
				vcfginternal_clear_key(section.keys[i]);
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
	inline void vcfg_clear() {
		if (m_configBuffer) {
			free((void*)m_configBuffer);
			m_configBuffer = 0;
		}

		if (m_sectionCount) {
			for (uint32_t i = 0; i < m_sectionCount; i++) {
				vcfginternal_clear_section(m_parsedData[i]);
			}

			free((void*)(m_parsedData));
			m_parsedData = 0;

			m_sectionCount = 0;
		}

		#if !defined(VCFG_BUFFER_ONLY)
			if (m_currentConfigFile) {
				fclose(m_currentConfigFile);
				m_currentConfigFile = 0;
			}
		#endif // VCFG_BUFFER_ONLY
	}



	/****************************************************/
	/*					Get functions					*/
	/****************************************************/

	/**
	 *	@brief Get section.
	 *
	 *	Returns the pointer to the section with the provided name
	 *
	 *	@param sectionName - name of the section (NULL for the root section)
	 *
	 *	@returns (TVCFGSection*) section pointer
	 */
	inline TVCFGSection* vcfg_get_section(const char* sectionName) {
		for (uint32_t i = 0; i < m_sectionCount; i++) {
			if (vcfginternal_strcmp(sectionName, m_parsedData[i].name) == 0) {
				return &(m_parsedData[i]);
			}
		}
		return 0;
	}

	/**
	 *	@brief Get string value from key.
	 *
	 *	Returns the value associated with the given key in the desired section
	 * 
	 *	@param sectionName - name of the section (NULL for the root section)
	 *	@param keyName - name of the key holding the desired value
	 * 
	 *	@returns (const char*) value of the given key
	 */
	inline const char* vcfg_get_string(const char* sectionName, const char* keyName) {
		TVCFGSection* section = vcfg_get_section(sectionName);

		for (uint32_t i = 0; i < section->keyCount; i++) {
			if (vcfginternal_strcmp(section->keys[i].name, keyName) == 0) {
				return section->keys[i].value;
			}
		}
		return 0;
	}

	/**
	 *	@brief Get integer value from key.
	 *
	 *	Returns the value associated with the given key in the desired section
	 *
	 *	@param sectionName - name of the section (NULL for the root section)
	 *	@param keyName - name of the key holding the desired value
	 *
	 *	@returns (int64_t) value of the given key
	 */
	inline int64_t vcfg_get_int(const char* sectionName, const char* keyName) {
		const char* stringValue = vcfg_get_string(sectionName, keyName);
		return vcfginternal_strtoint(stringValue);
	}

	/**
	 *	@brief Get floating point value from key.
	 *
	 *	Returns the value associated with the given key in the desired section
	 *
	 *	@param sectionName - name of the section (NULL for the root section)
	 *	@param keyName - name of the key holding the desired value
	 *
	 *	@returns (double) value of the given key
	 */
	inline double vcfg_get_float(const char* sectionName, const char* keyName) {
		const char* stringValue = vcfg_get_string(sectionName, keyName);
		return vcfginternal_strtofloat(stringValue);
	}

	/**
	 *	@brief Get boolean value (true|false) from key.
	 *
	 *	Returns the value associated with the given key in the desired section
	 *
	 *	@param sectionName - name of the section (NULL for the root section)
	 *	@param keyName - name of the key holding the desired value
	 *
	 *	@returns (int [1-true; 0-false]) value of the given key
	 */
	inline int vcfg_get_bool(const char* sectionName, const char* keyName) {
		const char* stringValue = vcfg_get_string(sectionName, keyName);
		return (vcfginternal_strcmp(stringValue, "true") == 0 ? 1 : 0);
	}

	/**
	 *	@brief Get key node.
	 *
	 *	Returns the entire node associated with the given key in the desired section
	 *
	 *	@param sectionName - name of the section (NULL for the root section)
	 *	@param keyName - name of the key node
	 *
	 *	@returns (const VCFG_Node*) entire node of the given key
	 */
	inline const VCFG_Node* vcfg_get_node(const char* sectionName, const char* keyName) {
		TVCFGSection* section = vcfg_get_section(sectionName);

		for (uint32_t i = 0; i < section->keyCount; i++) {
			if (vcfginternal_strcmp(section->keys[i].name, keyName) == 0) {
				return &(section->keys[i]);
			}
		}
		return 0;
	}

	/**
	 *	@brief Get key node from parent node.
	 *
	 *	Returns the entire node associated with the given key in the desired parent node
	 *
	 *	@param parentNode - node of the element to search in (NULL for the root section)
	 *	@param keyName - name of the key node
	 *
	 *	@returns (const VCFG_Node*) entire node of the given key
	 */
	inline const VCFG_Node* vcfg_get_node_from_node(const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_node(0, keyName);

		for (uint32_t i = 0; i < parentNode->childCount; i++) {
			if (vcfginternal_strcmp(parentNode->children[i].name, keyName) == 0) {
				return &(parentNode->children[i]);
			}
		}
		return 0;
	}

	/**
	 *	@brief Get string value from key inside the given node.
	 *
	 *	Returns the value associated with the given key in the desired node
	 *
	 *	@param parentNode - node of the element to search in (NULL for the root section)
	 *	@param keyName - name of the key holding the desired value
	 *
	 *	@returns (const char*) value of the given key
	 */
	inline const char* vcfg_get_string_from_node(const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_string(0, keyName);

		for (uint32_t i = 0; i < parentNode->childCount; i++) {
			if (vcfginternal_strcmp(parentNode->children[i].name, keyName) == 0) {
				return parentNode->children[i].value;
			}
		}
		return 0;
	}

	/**
	 *	@brief Get integer value from key inside the given node.
	 *
	 *	Returns the value associated with the given key in the desired node
	 *
	 *	@param parentNode - node of the element to search in (NULL for the root section)
	 *	@param keyName - name of the key holding the desired value
	 *
	 *	@returns (int64_t) value of the given key
	 */
	inline int64_t vcfg_get_int_from_node(const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_int(0, keyName);
		
		const char* stringValue = vcfg_get_string_from_node(parentNode, keyName);
		return vcfginternal_strtoint(stringValue);
	}

	/**
	 *	@brief Get floating point value from key inside the given node.
	 *
	 *	Returns the value associated with the given key in the desired node
	 *
	 *	@param parentNode - node of the element to search in (NULL for the root section)
	 *	@param keyName - name of the key holding the desired value
	 *
	 *	@returns (double) value of the given key
	 */
	inline double vcfg_get_float_from_node(const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_float(0, keyName);

		const char* stringValue = vcfg_get_string_from_node(parentNode, keyName);
		return vcfginternal_strtofloat(stringValue);
	}

	/**
	 *	@brief Get boolean value (true|false) from key inside the given node.
	 *
	 *	Returns the value associated with the given key in the desired node
	 *
	 *	@param parentNode - node of the element to search in (NULL for the root section)
	 *	@param keyName - name of the key holding the desired value
	 *
	 *	@returns (int [1-true; 0-false]) value of the given key
	 */
	inline int vcfg_get_bool_from_node(const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_bool(0, keyName);

		const char* stringValue = vcfg_get_string_from_node(parentNode, keyName);
		return (vcfginternal_strcmp(stringValue, "true") == 0 ? 1 : 0);
	}

#ifdef __cplusplus
}
#endif // __cplusplus

// C++ wrapper
#ifdef __cplusplus
	
#endif // __cplusplus

#endif // VORTEX_CONFIG_LIB