#include "Test.hpp"
#include "rlm3-string.h"
#include <string>


TEST_CASE(UIntToString_HappyCase)
{
	uint32_t TEST_CASES[] = { 0, 1, 9, 10, 11, 19, 20, 99, 100, 1000000000, 4294967295 };

	for (uint32_t test_case : TEST_CASES)
	{
		std::string expected = std::to_string(test_case);
		char buffer[15];
		for (size_t i = 0; i < sizeof(buffer); i++)
			buffer[i] = (int)i + 1;
		ASSERT(RLM3_UIntToString(test_case, buffer, expected.length() + 1));
		ASSERT(std::to_string(test_case) == buffer);
		for (size_t i = expected.length() + 1; i < sizeof(buffer); i++)
			ASSERT(buffer[i] == (int)i + 1);
	}
}

TEST_CASE(UIntToString_Overflow)
{
	uint32_t TEST_CASES[] = { 0, 1, 9, 10, 11, 19, 20, 99, 100, 1000000000, 4294967295 };

	for (uint32_t test_case : TEST_CASES)
	{
		std::string expected = std::to_string(test_case);
		char buffer[15];
		for (size_t i = 0; i < sizeof(buffer); i++)
			buffer[i] = (int)i + 1;
		ASSERT(!RLM3_UIntToString(test_case, buffer, expected.length()));
		for (size_t i = 0; i < sizeof(buffer); i++)
			ASSERT(buffer[i] == (int)i + 1);
	}
}

