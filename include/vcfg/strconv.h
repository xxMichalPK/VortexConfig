﻿/*
 * strconv.h
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

#ifndef VCFG_STRCONV_H
#define VCFG_STRCONV_H 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	#include <stdint.h>
	#include <stdlib.h>

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

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // VCFG_STRCONV_H