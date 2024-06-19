#ifndef VCFG_PARSER_H
#define VCFG_PARSER_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	#include <stdint.h>
	#include "compatibility.h"

	#if !defined(VCFG_BUFFER_ONLY)
		#include <stdio.h>
	#endif

	typedef struct VCFGKey {
		char* name;
		char* value;
		uint32_t childCount;
		struct VCFGKey* children;
	} VCFGKey_t;
	typedef VCFGKey_t VCFG_Node;

	typedef struct VCFGSection {
		char* name;
		uint32_t keyCount;
		VCFGKey_t* keys;
	} VCFGSection_t;

	typedef struct VCFGParser {
		#if !defined(VCFG_BUFFER_ONLY)
			FILE* m_currentConfigFile;
		#endif

		// The raw data of the configuration file
		const char* m_configBuffer;
		size_t m_configBufferLength;

		// The parsed data
		VCFGSection_t* m_parsedData;
		uint32_t m_sectionCount;
	} VCFGParser_t;
	typedef VCFGParser_t VCFG_Parser;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // VCFG_PARSER_H