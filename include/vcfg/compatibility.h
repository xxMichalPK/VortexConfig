#ifndef VCFG_COMPATIBILITY_H
#define VCFG_COMPATIBILITY_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	#include <stdint.h>
	#include "macros.h"

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

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // VCFG_COMPATIBILITY_H