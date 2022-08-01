#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*RLM3_Format_Fn)(void* data, char c);

extern void RLM3_FnFormat(RLM3_Format_Fn fn, void* data, const char* format, ...) __attribute__ ((format (printf, 3, 4)));
extern void RLM3_FnVFormat(RLM3_Format_Fn fn, void* data, const char* format, va_list args) __attribute__ ((format (printf, 3, 0)));

extern size_t RLM3_Format(char* buffer, size_t size, const char* format, ...) __attribute__ ((format (printf, 3, 4)));
extern size_t RLM3_FormatNoNul(char* buffer, size_t size, const char* format, ...) __attribute__ ((format (printf, 3, 4)));
extern size_t RLM3_VFormat(char* buffer, size_t size, const char* format, va_list args) __attribute__ ((format (printf, 3, 0)));
extern size_t RLM3_VFormatNoNul(char* buffer, size_t size, const char* format, va_list args) __attribute__ ((format (printf, 3, 0)));


#ifdef __cplusplus
}
#endif
