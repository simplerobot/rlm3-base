#include "Test.hpp"
#include "rlm3-string.h"
#include <string>


TEST_CASE(UIntToString_HappyCase)
{
	uint32_t TEST_CASES[][2] = {
			{ 0, 1 },
			{ 1, 1 },
			{ 9, 1 },
			{ 10, 2 },
			{ 11, 2 },
			{ 19, 2 },
			{ 20, 2 },
			{ 99, 2 },
			{ 100, 3 },
			{ 1000000000, 10 },
			{ 4294967295, 10 },
	};

	for (auto& test_case : TEST_CASES)
	{
		uint32_t test_value = test_case[0];
		uint32_t expected_length = test_case[1];

		char buffer[15];
		for (size_t i = 0; i < sizeof(buffer); i++)
			buffer[i] = (int)i + 1;

		ASSERT(RLM3_UIntToString(test_value, buffer, expected_length + 1));

		ASSERT(std::stoul(buffer) == test_value);
		for (size_t i = expected_length + 1; i < sizeof(buffer); i++)
			ASSERT(buffer[i] == (int)i + 1);
	}
}

TEST_CASE(UIntToString_Overflow)
{
	uint32_t TEST_CASES[][2] = {
			{ 0, 1 },
			{ 1, 1 },
			{ 9, 1 },
			{ 10, 2 },
			{ 11, 2 },
			{ 19, 2 },
			{ 20, 2 },
			{ 99, 2 },
			{ 100, 3 },
			{ 1000000000, 10 },
			{ 4294967295, 10 },
	};


	for (auto& test_case : TEST_CASES)
	{
		uint32_t test_value = test_case[0];
		uint32_t expected_length = test_case[1];

		char buffer[15];
		for (size_t i = 0; i < sizeof(buffer); i++)
			buffer[i] = (int)i + 1;

		ASSERT(!RLM3_UIntToString(test_value, buffer, expected_length));

		for (size_t i = 0; i < sizeof(buffer); i++)
			ASSERT(buffer[i] == (int)i + 1);
	}
}

