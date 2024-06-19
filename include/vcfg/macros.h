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