#include "rlm3-string.h"


extern bool RLM3_UIntToString(uint32_t value, char* result, size_t size)
{
	uint32_t count = 1;
	for (uint32_t x = value; x / 10 > 0; x /= 10)
		count++;
	if (size < count + 1)
		return false;
	result[count] = 0;
	for (uint32_t x = value; count > 0; x /= 10)
		result[--count] = '0' + x % 10;
	return true;
}

