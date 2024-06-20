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

#ifndef VORTEX_CONFIG_H
#define VORTEX_CONFIG_H 1

#include "macros.h"
#include "parser.h"
#include "strconv.h"

// All the necessary C code
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	// We need stdlib.h for malloc, calloc, realloc and free.
	// Those can be replaced by a custom implementation in an embedded system
	#include <stdlib.h>

	/**
	 *	@brief Set config buffer.
	 *	
	 *	Sets the raw data buffer to the specified input buffer
	 * 
	 *	@param inputBuffer - the desired raw data buffer
	 *	@param dataLength - length of the input
	 */
	inline void vcfg_set_buffer(VCFG_Parser* parserObj, const char* inputBuffer, size_t dataLength) {
		// If the buffer was already set try to free it
		if (parserObj->m_configBuffer) {
			free((void*)(parserObj->m_configBuffer));
			parserObj->m_configBuffer = 0;
		}

		parserObj->m_configBuffer = inputBuffer;
		parserObj->m_configBufferLength = dataLength;
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
	inline size_t vcfginternal_skipwhitespace(VCFG_Parser* parserObj, const char **dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;
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
	inline size_t vcfginternal_skiplinecomment(VCFG_Parser* parserObj, const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		// Check if we have a comment "//"
		if ((*internalDataPtr != '/') || (*(internalDataPtr + 1) != '/')) return 0;

		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;
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
	inline size_t vcfginternal_skipblockcomment(VCFG_Parser* parserObj, const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		// Check if we have a comment block "/*"
		if ((*internalDataPtr != '/') || (*(internalDataPtr + 1) != '*')) return 0;
		
		// Skip the first comment block characters to avoid something like this: "/*/"
		internalDataPtr += 2;

		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;
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
	inline size_t vcfginternal_skipcomments(VCFG_Parser* parserObj, const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		size_t skippedCount = 0;
		skippedCount += vcfginternal_skiplinecomment(parserObj, &internalDataPtr);
		skippedCount += vcfginternal_skipblockcomment(parserObj, &internalDataPtr);

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
	inline size_t vcfginternal_createsection(VCFG_Parser* parserObj, const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		if (*internalDataPtr != '[') return 0;

		// If we have a [ it means we have to create a new section that we "push" on top
		// of the previously added section (for that reason section names have to be unique)
		++internalDataPtr;
		size_t nameLength = 0;
		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;
		while ((*internalDataPtr != ']') && (internalDataPtr < dataEndPtr)) {
			++internalDataPtr;
			++nameLength;
		}
		if (*internalDataPtr == ']') ++internalDataPtr;
		size_t skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;

		// We don't allow empty sections -> []
		if (nameLength == 0) return skippedCount;

		++(parserObj->m_sectionCount);
		VCFGSection_t* newSections = (VCFGSection_t*)realloc(parserObj->m_parsedData, (parserObj->m_sectionCount) * sizeof(VCFGSection_t));
		if (!newSections) {
			--(parserObj->m_sectionCount);
			return skippedCount;
		}

		newSections[(parserObj->m_sectionCount) - 1].keyCount = 0;
		newSections[(parserObj->m_sectionCount) - 1].keys = 0;
		newSections[(parserObj->m_sectionCount) - 1].name = (char*)malloc(nameLength + 1);
		if (!(newSections[(parserObj->m_sectionCount) - 1].name)) {
			--(parserObj->m_sectionCount);
			return skippedCount;
		}

		vcfginternal_memcpy((void*)(newSections[(parserObj->m_sectionCount) - 1].name), (void*)(internalDataPtr - nameLength - 1), nameLength);
		newSections[(parserObj->m_sectionCount) - 1].name[nameLength] = '\0';

		parserObj->m_parsedData = newSections;

		return skippedCount;
	}

	// Forward declare the needed function
	inline size_t vcfginternal_parsearray_keyvalue(VCFG_Parser* parserObj, const char** dataPtr, size_t valueIndex, VCFGKey_t* keyValuePair);
	inline size_t vcfginternal_parsearray(VCFG_Parser* parserObj, const char** dataPtr, VCFGKey_t* keyValuePair) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		if (*internalDataPtr != '[') return 0;
		++internalDataPtr;

		keyValuePair->value = (char*)malloc(8 * sizeof(char));
		if (keyValuePair->value) {
			vcfginternal_memcpy((void*)(keyValuePair->value), (void*)"[array]\0", 8);
		}
		
		vcfginternal_skipwhitespace(parserObj, &internalDataPtr);

		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;
		size_t arrayIndex = 0;
		while ((internalDataPtr < dataEndPtr) && (*internalDataPtr != ']')) {
			// Skip all whitespaces and comments
			if (vcfginternal_skipwhitespace(parserObj, &internalDataPtr)) continue;
			if (vcfginternal_skipcomments(parserObj, &internalDataPtr)) continue;

			if (vcfginternal_parsearray_keyvalue(parserObj, &internalDataPtr, arrayIndex, keyValuePair)) {
				vcfginternal_skipwhitespace(parserObj, &internalDataPtr);
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
	inline size_t vcfginternal_parseobject_keyvalue(VCFG_Parser* parserObj, const char** dataPtr, VCFGKey_t* keyValuePair);
	inline size_t vcfginternal_parseobject(VCFG_Parser* parserObj, const char** dataPtr, VCFGKey_t* keyValuePair) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		if (*internalDataPtr != '{') return 0;
		++internalDataPtr;

		keyValuePair->value = (char*)malloc(9 * sizeof(char));
		if (keyValuePair->value) {
			vcfginternal_memcpy((void*)(keyValuePair->value), (void*)"{object}\0", 9);
		}

		vcfginternal_skipwhitespace(parserObj, &internalDataPtr);

		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;
		while ((internalDataPtr < dataEndPtr) && (*internalDataPtr != '}')) {
			// Skip all whitespaces and comments
			if (vcfginternal_skipwhitespace(parserObj, &internalDataPtr)) continue;
			if (vcfginternal_skipcomments(parserObj, &internalDataPtr)) continue;

			if (vcfginternal_parseobject_keyvalue(parserObj, &internalDataPtr, keyValuePair)) {
				vcfginternal_skipwhitespace(parserObj, &internalDataPtr);
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
	inline size_t vcfginternal_parsevalue(VCFG_Parser* parserObj, const char** dataPtr, VCFGKey_t *keyValuePair) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		int valueInQuotes = (*internalDataPtr == '"') ? 1 : 0;
		if (valueInQuotes) ++internalDataPtr;

		size_t valueLength = 0;
		const char* valueStart = internalDataPtr;
		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;
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

	inline size_t vcfginternal_parsearray_keyvalue(VCFG_Parser* parserObj, const char** dataPtr, size_t valueIndex, VCFGKey_t* keyValuePair) {
		if (!dataPtr || !keyValuePair) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		char* keyName = vcfginternal_unsignednumtostr(valueIndex);
		if (!keyName) return 0;

		vcfginternal_skipwhitespace(parserObj, &internalDataPtr);

		// Don't allow empty keys
		size_t skippedCount = internalDataPtr - *dataPtr;

		// Add the key to the parent key
		keyValuePair->childCount++;
		VCFGKey_t* newChildren = (VCFGKey_t*)realloc((void*)(keyValuePair->children), keyValuePair->childCount * sizeof(VCFGKey_t));
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

		vcfginternal_skipwhitespace(parserObj, &internalDataPtr);

		// Check if the value is an array or object
		if (*internalDataPtr == '{') {
			vcfginternal_parseobject(parserObj, &internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}
		else if (*internalDataPtr == '[') {
			vcfginternal_parsearray(parserObj, &internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}
		else {
			vcfginternal_parsevalue(parserObj, &internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}

		skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	inline size_t vcfginternal_parseobject_keyvalue(VCFG_Parser* parserObj, const char** dataPtr, VCFGKey_t* keyValuePair) {
		if (!dataPtr || !keyValuePair) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		int keyInQuotes = (*internalDataPtr == '"') ? 1 : 0;
		if (keyInQuotes) ++internalDataPtr;

		size_t keyLength = 0;
		const char* keyStart = internalDataPtr;
		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;
		while ((internalDataPtr < dataEndPtr)) {
			if (keyInQuotes && (*internalDataPtr == '"')) {
				++internalDataPtr;
				break;
			}
			if (!keyInQuotes && (VCFG_IS_WHITESPACE(*internalDataPtr) || (*internalDataPtr == '='))) break;

			++internalDataPtr;
			++keyLength;
		}

		vcfginternal_skipwhitespace(parserObj, &internalDataPtr);

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
		VCFGKey_t* newChildren = (VCFGKey_t*)realloc((void*)(keyValuePair->children), keyValuePair->childCount * sizeof(VCFGKey_t));
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

		vcfginternal_skipwhitespace(parserObj, &internalDataPtr);

		// Check if the value is an array or object
		if (*internalDataPtr == '{') {
			vcfginternal_parseobject(parserObj, &internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}
		else if (*internalDataPtr == '[') {
			vcfginternal_parsearray(parserObj, &internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}
		else {
			vcfginternal_parsevalue(parserObj, &internalDataPtr, &(newChildren[keyValuePair->childCount - 1]));
		}

		skippedCount = internalDataPtr - *dataPtr;
		*dataPtr = internalDataPtr;
		return skippedCount;
	}

	inline size_t vcfginternal_parsekeyvalue(VCFG_Parser* parserObj, const char** dataPtr) {
		if (!dataPtr) return 0;

		const char* internalDataPtr = *dataPtr;
		if (!internalDataPtr) return 0;

		// We support keys in quotes so we need to account for that.
		int keyInQuotes = (*internalDataPtr == '"') ? 1 : 0;
		if (keyInQuotes) ++internalDataPtr;

		size_t keyLength = 0;
		const char* keyStart = internalDataPtr;
		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;
		while ((internalDataPtr < dataEndPtr)) {
			if (keyInQuotes && (*internalDataPtr == '"')) { 
				++internalDataPtr;
				break;
			}
			if (!keyInQuotes && (VCFG_IS_WHITESPACE(*internalDataPtr) || (*internalDataPtr == '='))) break;

			++internalDataPtr;
			++keyLength;
		}

		vcfginternal_skipwhitespace(parserObj, &internalDataPtr);

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
		parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount++;
		VCFGKey_t* newKeys = (VCFGKey_t*)realloc((void*)(parserObj->m_parsedData[parserObj->m_sectionCount - 1].keys), parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount * sizeof(VCFGKey_t));
		if (!newKeys) {
			parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount--;
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1].childCount = 0;
		newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1].children = 0;
		newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1].value = 0;
		newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1].name = (char*)malloc(keyLength + 1);
		if (!(newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1].name)) {
			parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount--;
			*dataPtr = internalDataPtr;
			return skippedCount;
		}
		vcfginternal_memcpy((void*)(newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1].name), (void*)keyStart, keyLength);
		newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1].name[keyLength] = '\0';

		parserObj->m_parsedData[parserObj->m_sectionCount - 1].keys = newKeys;

		// Skip the equal sign
		++internalDataPtr;

		vcfginternal_skipwhitespace(parserObj, &internalDataPtr);

		// Check if the value is an array or object
		if (*internalDataPtr == '{') {
			vcfginternal_parseobject(parserObj, &internalDataPtr, &(newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1]));
		}
		else if (*internalDataPtr == '[') {
			vcfginternal_parsearray(parserObj, &internalDataPtr, &(newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1]));
		}
		else {
			vcfginternal_parsevalue(parserObj, &internalDataPtr, &(newKeys[parserObj->m_parsedData[parserObj->m_sectionCount - 1].keyCount - 1]));
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
	inline int vcfg_parse(VCFG_Parser* parserObj) {
		if (!(parserObj->m_configBuffer) || !(parserObj->m_configBufferLength)) return 0;

		// We need to store the end of the buffer to avoid overflowing
		const char* dataEndPtr = parserObj->m_configBuffer + parserObj->m_configBufferLength;

		// We work on a temporary variable to ensure the original pointer is in tact
		const char* internalDataPtr = parserObj->m_configBuffer;

		// Allocate buffer for the root section
		parserObj->m_parsedData = (VCFGSection_t*)calloc(1, sizeof(VCFGSection_t));
		if (!(parserObj->m_parsedData)) return 0;
		parserObj->m_sectionCount = 1;

		// TODO:
		//	- Maybe a better error handling?
		//	- Test for edge cases
		while (internalDataPtr < dataEndPtr) {
			// Skip all whitespaces and comments
			if (vcfginternal_skipwhitespace(parserObj, &internalDataPtr)) continue;
			if (vcfginternal_skipcomments(parserObj, &internalDataPtr)) continue;

			// If there is a section -> [sectionname] -> create a new section
			// in the parsed data structure and push it to the end
			if (vcfginternal_createsection(parserObj, &internalDataPtr)) continue;

			// The only thing left to do is to parse the key-value pairs
			if (vcfginternal_parsekeyvalue(parserObj, &internalDataPtr)) continue;
			
			// If everything returned with 0 (no data was parsed) just break
			break;
		}

		return 1;
	}

	// For embedded systems we can avoid using file operations and use only statically defined buffers
	// so in case someone doesn't have the file operations implemented we use preprocessor definitions
	#if !defined(VCFG_BUFFER_ONLY)
	#include <stdio.h>

	/**
	 *	@brief Open configuration file.
	 * 
	 *	Opens the configuration file and reads the content to the internal buffer
	 * 
	 *	@param s_path - path to the configuration file
	 * 
	 *	@returns 0 - Failure, 1 - Success
	 */
	inline int vcfg_open(VCFG_Parser* parserObj, const char *s_path) {
		// If a file was already opened close the previous one to avoid memory leaks
		if (parserObj->m_currentConfigFile) {
			fclose(parserObj->m_currentConfigFile);
			parserObj->m_currentConfigFile = 0;
		}

		// Open the desired file
		errno_t fileOpenFailed = fopen_s(&(parserObj->m_currentConfigFile), s_path, "rb");
		if (fileOpenFailed || !(parserObj->m_currentConfigFile)) {
			perror("FOPEN()");
			return 0; // Failure
		}

		// Create a buffer for the file to read to
		fseek(parserObj->m_currentConfigFile, 0, SEEK_END);
		size_t fileSize = ftell(parserObj->m_currentConfigFile);
		fseek(parserObj->m_currentConfigFile, 0, SEEK_SET);

		// Clear the previously used buffer
		if (parserObj->m_configBuffer) {
			free((void*)(parserObj->m_configBuffer));
			parserObj->m_configBuffer = 0;
		}

		// Allocate memory
		parserObj->m_configBuffer = (char*)malloc((fileSize + 1) * sizeof(char));
		if (!(parserObj->m_configBuffer)) {
			perror("MALLOC()");
			return 0;
		}

		// Read the file contents into the buffer
		if (fread((void*)(parserObj->m_configBuffer), sizeof(char), fileSize, parserObj->m_currentConfigFile) != fileSize) {
			perror("FREAD()");
			return 0;
		};

		parserObj->m_configBufferLength = fileSize;
		
		return vcfg_parse(parserObj);
	}
	#endif // VCFG_BUFFER_ONLY

	/**
	 *	@brief Clear key.
	 *
	 *	Clears the entire key by freeing the allocated memory and recursively calling
	 *	itself for all the nested child keys
	 */
	inline void vcfginternal_clear_key(VCFGKey_t key) {
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
	inline void vcfginternal_clear_section(VCFGSection_t section) {
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
	inline void vcfg_clear(VCFG_Parser* parserObj) {
		if (parserObj->m_configBuffer) {
			free((void*)(parserObj->m_configBuffer));
			parserObj->m_configBuffer = 0;
		}

		if (parserObj->m_sectionCount) {
			for (uint32_t i = 0; i < parserObj->m_sectionCount; i++) {
				vcfginternal_clear_section(parserObj->m_parsedData[i]);
			}

			free((void*)(parserObj->m_parsedData));
			parserObj->m_parsedData = 0;

			parserObj->m_sectionCount = 0;
		}

		#if !defined(VCFG_BUFFER_ONLY)
			if (parserObj->m_currentConfigFile) {
				fclose(parserObj->m_currentConfigFile);
				parserObj->m_currentConfigFile = 0;
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
	 *	@returns (VCFGSection_t*) section pointer
	 */
	inline VCFGSection_t* vcfg_get_section(VCFG_Parser* parserObj, const char* sectionName) {
		for (uint32_t i = 0; i < parserObj->m_sectionCount; i++) {
			if (vcfginternal_strcmp(sectionName, parserObj->m_parsedData[i].name) == 0) {
				return &(parserObj->m_parsedData[i]);
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
	inline const char* vcfg_get_string(VCFG_Parser* parserObj, const char* sectionName, const char* keyName) {
		VCFGSection_t* section = vcfg_get_section(parserObj, sectionName);

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
	inline int64_t vcfg_get_int(VCFG_Parser* parserObj, const char* sectionName, const char* keyName) {
		const char* stringValue = vcfg_get_string(parserObj, sectionName, keyName);
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
	inline double vcfg_get_float(VCFG_Parser* parserObj, const char* sectionName, const char* keyName) {
		const char* stringValue = vcfg_get_string(parserObj, sectionName, keyName);
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
	inline int vcfg_get_bool(VCFG_Parser* parserObj, const char* sectionName, const char* keyName) {
		const char* stringValue = vcfg_get_string(parserObj, sectionName, keyName);
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
	inline const VCFG_Node* vcfg_get_node(VCFG_Parser* parserObj, const char* sectionName, const char* keyName) {
		VCFGSection_t* section = vcfg_get_section(parserObj, sectionName);

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
	inline const VCFG_Node* vcfg_get_node_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_node(parserObj, 0, keyName);

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
	inline const char* vcfg_get_string_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_string(parserObj, 0, keyName);

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
	inline int64_t vcfg_get_int_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_int(parserObj, 0, keyName);
		
		const char* stringValue = vcfg_get_string_from_node(parserObj, parentNode, keyName);
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
	inline double vcfg_get_float_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_float(parserObj, 0, keyName);

		const char* stringValue = vcfg_get_string_from_node(parserObj, parentNode, keyName);
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
	inline int vcfg_get_bool_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName) {
		// If there's no parent use the root section as parent
		if (!parentNode) return vcfg_get_bool(parserObj, 0, keyName);

		const char* stringValue = vcfg_get_string_from_node(parserObj, parentNode, keyName);
		return (vcfginternal_strcmp(stringValue, "true") == 0 ? 1 : 0);
	}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // VORTEX_CONFIG_H