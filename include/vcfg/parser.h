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

	#ifndef __cplusplus
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
	#endif

	// Forward declarations of the C and C++ shared functions and structures
	typedef struct VCFGParser VCFG_Parser;

	#if !defined(VCFG_BUFFER_ONLY)
		inline int vcfg_open(VCFG_Parser* parserObj, const char* s_path);
	#endif
	inline void vcfg_clear(VCFG_Parser* parserObj);
	inline void vcfg_set_buffer(VCFG_Parser* parserObj, const char* inputBuffer, size_t dataLength);
	inline int vcfg_parse(VCFG_Parser* parserObj);


	inline VCFGSection_t* vcfg_get_section(VCFG_Parser* parserObj, const char* sectionName);
	
	inline const char* vcfg_get_string(VCFG_Parser* parserObj, const char* sectionName, const char* keyName);
	inline const char* vcfg_get_string_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName);
	
	inline int64_t vcfg_get_int(VCFG_Parser* parserObj, const char* sectionName, const char* keyName);
	inline int64_t vcfg_get_int_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName);
	
	inline double vcfg_get_float(VCFG_Parser* parserObj, const char* sectionName, const char* keyName);
	inline double vcfg_get_float_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName);
	
	inline int vcfg_get_bool(VCFG_Parser* parserObj, const char* sectionName, const char* keyName);
	inline int vcfg_get_bool_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName);
	
	inline const VCFG_Node* vcfg_get_node(VCFG_Parser* parserObj, const char* sectionName, const char* keyName);
	inline const VCFG_Node* vcfg_get_node_from_node(VCFG_Parser* parserObj, const VCFG_Node* parentNode, const char* keyName);

#ifdef __cplusplus
}
#endif // __cplusplus

// The C++ wrapper for the C functions
#ifdef __cplusplus
	class VCFGParser {
		// TODO:
		//   Change those variables to private ones
		public:	// It has to be public for backwards compatibility for now
			#if !defined(VCFG_BUFFER_ONLY)
				FILE* m_currentConfigFile = nullptr;
			#endif

			// The raw data of the configuration file
			const char* m_configBuffer = nullptr;
			size_t m_configBufferLength = 0;

			// The parsed data
			VCFGSection_t* m_parsedData = nullptr;
			uint32_t m_sectionCount = 0;

		public:
			VCFGParser() {}
			~VCFGParser() { vcfg_clear(this); }

			#if !defined(VCFG_BUFFER_ONLY)
				/**
				 *	@brief Open configuration file.
				 * 
				 *	Opens and parses the specified configuration file
				 * 
				 *	@param path - the path of the configuration file
				 * 
				 *	@returns 0 - Failure, 1 - Success
				 */
				int Open(const char* path) { return vcfg_open(this, path); }
			#endif

			/**
			 *	@brief Clear the parser
			 * 
			 *	Deallocates all reserved memory and closes the configuration file
			 */
			void Clear() { vcfg_clear(this); }

			/**
			 *	@brief Set parser buffer.
			 * 
			 *	Sets the buffer which the parser works on
			 * 
			 *	@param inputBuffer - the buffer containing raw configuration data
			 *	@param dataLength - length of the data
			 */
			void SetBuffer(const char* inputBuffer, size_t dataLength) { vcfg_set_buffer(this, inputBuffer, dataLength); }

			/**
			 *	@brief Parse configuration.
			 *
			 *	Parses the configuration data stored in the raw data buffer.
			 *	It has to be called explicitly when using a custom buffer
			 *
			 *	@returns 0 - Failure, 1 - Success
			 */
			int Parse() { return vcfg_parse(this); }

			/**
			 *	@brief Read string from configuration.
			 * 
			 *	Returns the string value associated with the specified key
			 * 
			 *	@param sectionName / parentNode - the name of the section containing the key / the node holding the key
			 *	@param keyName - name of the key
			 * 
			 *	@returns (const char*) - the string value associated with the key
			 */
			const char* GetString(const char* keyName) { return vcfg_get_string(this, nullptr, keyName); }
			const char* GetString(const char* sectionName, const char* keyName) { return vcfg_get_string(this, sectionName, keyName); }
			const char* GetString(const VCFG_Node* parentNode, const char* keyName) { return vcfg_get_string_from_node(this, parentNode, keyName); }

			/**
			 *	@brief Read integer from configuration.
			 *
			 *	Returns the 64bit integer value associated with the specified key
			 *
			 *	@param sectionName / parentNode - the name of the section containing the key / the node holding the key
			 *	@param keyName - name of the key
			 *
			 *	@returns (int64_t) - the integer value associated with the key
			 */
			int64_t GetInt(const char* keyName) { return vcfg_get_int(this, nullptr, keyName); }
			int64_t GetInt(const char* sectionName, const char* keyName) { return vcfg_get_int(this, sectionName, keyName); }
			int64_t GetInt(const VCFG_Node* parentNode, const char* keyName) { return vcfg_get_int_from_node(this, parentNode, keyName); }

			/**
			 *	@brief Read floating point number from configuration.
			 *
			 *	Returns the floating point number value associated with the specified key
			 *
			 *	@param sectionName / parentNode - the name of the section containing the key / the node holding the key
			 *	@param keyName - name of the key
			 *
			 *	@returns (double) - the floating point number value associated with the key
			 */
			double GetFloat(const char* keyName) { return vcfg_get_float(this, nullptr, keyName); }
			double GetFloat(const char* sectionName, const char* keyName) { return vcfg_get_float(this, sectionName, keyName); }
			double GetFloat(const VCFG_Node* parentNode, const char* keyName) { return vcfg_get_float_from_node(this, parentNode, keyName); }

			/**
			 *	@brief Read boolean from configuration.
			 *
			 *	Returns the boolean value associated with the specified key
			 *
			 *	@param sectionName / parentNode - the name of the section containing the key / the node holding the key
			 *	@param keyName - name of the key
			 *
			 *	@returns (bool) - the boolean value associated with the key
			 */
			bool GetBool(const char* keyName) { return vcfg_get_bool(this, nullptr, keyName); }
			bool GetBool(const char* sectionName, const char* keyName) { return vcfg_get_bool(this, sectionName, keyName); }
			bool GetBool(const VCFG_Node* parentNode, const char* keyName) { return vcfg_get_bool_from_node(this, parentNode, keyName); }
			
			/**
			 *	@brief Read entire key node from configuration.
			 *
			 *	Returns the pointer to the specified key node
			 *
			 *	@param sectionName / parentNode - the name of the section containing the key / the node holding the key
			 *	@param keyName - name of the key
			 *
			 *	@returns (const VCFG_Node*) - the pointer to the specified key node
			 */
			const VCFG_Node* GetNode(const char* keyName) { return vcfg_get_node(this, nullptr, keyName); }
			const VCFG_Node* GetNode(const char* sectionName, const char* keyName) { return vcfg_get_node(this, sectionName, keyName); }
			const VCFG_Node* GetNode(const VCFG_Node* parentNode, const char* keyName) { return vcfg_get_node_from_node(this, parentNode, keyName); }

	};
	typedef VCFGParser VCFG_Parser;
#endif // __cplusplus

#endif // VCFG_PARSER_H