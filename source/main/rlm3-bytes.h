#pragma once

#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

	inline uint16_t hton_u16(uint16_t x) { return x; }
	inline uint16_t ntoh_u16(uint16_t x) { return x; }
	inline int16_t  hton_i16(int16_t x)  { return x; }
	inline int16_t  ntoh_i16(int16_t x)  { return x; }
	inline uint32_t hton_u32(uint32_t x) { return x; }
	inline uint32_t ntoh_u32(uint32_t x) { return x; }
	inline int32_t  hton_i32(int32_t x)  { return x; }
	inline int32_t  ntoh_i32(int32_t x)  { return x; }
	inline float    hton_f32(float x)    { return x; }
	inline float    ntoh_f32(float x)    { return x; }
	inline double   hton_f64(double x)   { return x; }
	inline double   ntoh_f64(double x)   { return x; }

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	inline uint16_t hton_u16(uint16_t x) { return __builtin_bswap16(x); }
	inline uint16_t ntoh_u16(uint16_t x) { return __builtin_bswap16(x); }
	inline int16_t  hton_i16(int16_t x)  { return __builtin_bswap16(x); }
	inline int16_t  ntoh_i16(int16_t x)  { return __builtin_bswap16(x); }
	inline uint32_t hton_u32(uint32_t x) { return __builtin_bswap32(x); }
	inline uint32_t ntoh_u32(uint32_t x) { return __builtin_bswap32(x); }
	inline int32_t  hton_i32(int32_t x)  { return __builtin_bswap32(x); }
	inline int32_t  ntoh_i32(int32_t x)  { return __builtin_bswap32(x); }
	inline float    hton_f32(float x)    { uint32_t result = __builtin_bswap32(*(uint32_t*)&x); return *(float*)&result; }
	inline float    ntoh_f32(float x)    { uint32_t result = __builtin_bswap32(*(uint32_t*)&x); return *(float*)&result; }
	inline double   hton_f64(double x)   { uint64_t result = __builtin_bswap64(*(uint64_t*)&x); return *(double*)&result; }
	inline double   ntoh_f64(double x)   { uint64_t result = __builtin_bswap64(*(uint64_t*)&x); return *(double*)&result; }

#else
	#error Unsupported Byte Order
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

	inline uint16_t hton(uint16_t x) { return hton_u16(x); }
	inline uint16_t ntoh(uint16_t x) { return ntoh_u16(x); }
	inline int16_t  hton(int16_t x)  { return hton_i16(x); }
	inline int16_t  ntoh(int16_t x)  { return ntoh_i16(x); }
	inline uint32_t hton(uint32_t x) { return hton_u32(x); }
	inline uint32_t ntoh(uint32_t x) { return ntoh_u32(x); }
	inline int32_t  hton(int32_t x)  { return hton_i32(x); }
	inline int32_t  ntoh(int32_t x)  { return ntoh_i32(x); }
	inline float    hton(float x)    { return hton_f32(x); }
	inline float    ntoh(float x)    { return ntoh_f32(x); }
	inline double   hton(double x)   { return hton_f64(x); }
	inline double   ntoh(double x)   { return ntoh_f64(x); }

#endif
