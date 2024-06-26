/*
 * macros.h
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

#ifndef VCFG_MACROS_H
#define VCFG_MACROS_H 1

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define OS_WINDOWS 1
#elif defined(__linux__)
	#define OS_LINUX 1
#endif

#ifndef VCFG_IS_WHITESPACE
	#define VCFG_IS_WHITESPACE(ch) ((ch == ' ') || ((ch >= '\b') && (ch <= '\r')))
#endif

#ifndef VCFG_IS_NUMBER
	#define VCFG_IS_NUMBER(ch) ((ch >= '0') && (ch <= '9'))
#endif

#endif // VCFG_MACROS_H