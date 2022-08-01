#include "Test.hpp"
#include "rlm3-string.h"
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <math.h>


const char* g_our_private_null_ptr = NULL;


TEST_CASE(VFormat_DirectCharacters_HappyCase)
{
	char buffer[1024];

	size_t size = RLM3_Format(buffer, sizeof(buffer), "Hello World");

	ASSERT(size == 12);
	ASSERT(std::strcmp(buffer, "Hello World") == 0);
}

TEST_CASE(VFormat_DirectCharacters_NULL)
{
	char buffer[1024];

	size_t size = RLM3_Format(buffer, sizeof(buffer), NULL);

	ASSERT(size == 7);
	ASSERT(std::strcmp(buffer, "<NULL>") == 0);
}

TEST_CASE(VFormat_DirectCharacters_EscapeCharacters)
{
	char buffer[1024];

	size_t size = RLM3_Format(buffer, sizeof(buffer), "27%%");

	ASSERT(size == 4);
	ASSERT(std::strcmp(buffer, "27%") == 0);
}

TEST_CASE(VFormat_DirectCharacters_Truncate)
{
	char buffer[1024];

	size_t size = RLM3_Format(buffer, 8, "Hello World");

	ASSERT(size == 12);
	ASSERT(std::strcmp(buffer, "Hello W") == 0);
}

TEST_CASE(VFormat_DirectCharacters_TruncateNoNul)
{
	char buffer[1024];
	for (auto& c : buffer)
		c = 'Z';

	size_t size = RLM3_FormatNoNul(buffer, 8, "Hello World");

	ASSERT(size == 11);
	ASSERT(std::strncmp(buffer, "Hello Wo", 8) == 0);
	ASSERT(std::strncmp(buffer, "Hello WoZ", 9) == 0);
}

TEST_CASE(VFormat_DirectCharacters_SpecialCharacters)
{
	char buffer[1024];

	size_t size = RLM3_Format(buffer, sizeof(buffer), "Hello World\\\r\n\xD7");

	ASSERT(size == 22);
	ASSERT(std::strcmp(buffer, "Hello World\\\\\\r\\n\\xD7") == 0);
}

TEST_CASE(VFormat_FormatCharacter_HappyCase)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%c%c%c", 'A', 'B', 'C');

	ASSERT(std::strcmp(buffer, "ABC") == 0);
}

TEST_CASE(VFormat_FormatCharacter_SpecialCharacters)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%c%c%c%c", '\\', '\r', '\n', 0xD7);

	ASSERT(std::strcmp(buffer, "\\\\\\r\\n\\xD7") == 0);
}

TEST_CASE(VFormat_FormatString_HappyCase)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%s", "ABC");

	ASSERT(std::strcmp(buffer, "ABC") == 0);
}

TEST_CASE(VFormat_FormatString_NULL)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%s", g_our_private_null_ptr);

	ASSERT(std::strcmp(buffer, "<NULL>") == 0);
}

TEST_CASE(VFormat_FormatString_Unaligned)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%s", (const char*)0x00000001);

	ASSERT(std::strcmp(buffer, "<MISALIGNED>") == 0);
}

TEST_CASE(VFormat_FormatString_SpecialCharacters)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%s", "Hello World\\\r\n\xD7");

	ASSERT(std::strcmp(buffer, "Hello World\\\\\\r\\n\\xD7") == 0);
}

TEST_CASE(VFormat_FormatUInt_HappyCase)
{
	uint32_t TEST_CASES[] = { 0, 1, 9, 10, 11, 19, 20, 99, 100, 1000000000, 4294967295 };

	for (uint32_t test_case : TEST_CASES)
	{
		char buffer[1024];

		RLM3_Format(buffer, sizeof(buffer), "%u", test_case);

		std::string expected = std::to_string(test_case);
		ASSERT(buffer == expected);
	}
}

TEST_CASE(VFormat_FormatInt_HappyCase)
{
	int32_t TEST_CASES[] = { 0, 1, 9, 10, 11, 19, 20, 99, 100, 1000000000, 2147483647, -0, -1, -9, -10, -1000000000, -2147483648 };

	for (int32_t test_case : TEST_CASES)
	{
		char buffer[1024];

		RLM3_Format(buffer, sizeof(buffer), "%d", test_case);

		std::string expected = std::to_string(test_case);
		ASSERT(buffer == expected);
	}
}

TEST_CASE(VFormat_FormatHexUpper_HappyCase)
{
	uint32_t TEST_CASES[] = { 0, 1, 9, 10, 11, 15, 16, 19, 20, 99, 100, 1000000000, 4294967295 };

	for (uint32_t test_case : TEST_CASES)
	{
		char buffer[1024];

		RLM3_Format(buffer, sizeof(buffer), "%X", test_case);

		std::ostringstream out;
		out << std::hex << test_case;
		std::string expected = out.str();
		for (auto& c : expected)
			c = std::toupper(c);

		ASSERT(buffer == expected);
	}
}

TEST_CASE(VFormat_FormatHexLower_HappyCase)
{
	uint32_t TEST_CASES[] = { 0, 1, 9, 10, 11, 15, 16, 19, 20, 99, 100, 1000000000, 4294967295 };

	for (uint32_t test_case : TEST_CASES)
	{
		char buffer[1024];

		RLM3_Format(buffer, sizeof(buffer), "%x", test_case);

		std::ostringstream out;
		out << std::hex << test_case;
		std::string expected = out.str();
		for (auto& c : expected)
			c = std::tolower(c);

		ASSERT(buffer == expected);
	}
}

TEST_CASE(VFormat_FormatFloat_HappyCase)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%f %f", -123.456, M_PI);

	ASSERT(std::strcmp(buffer, "-123.456 3.141593") == 0);
}

TEST_CASE(VFormat_FormatFloat_NotANumber)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%f %f", -std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());

	ASSERT(std::strcmp(buffer, "-nan nan") == 0);
}

TEST_CASE(VFormat_FormatFloat_Infinity)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%f %f", -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());

	ASSERT(std::strcmp(buffer, "-inf inf") == 0);
}

TEST_CASE(VFormat_FormatFloat_Zero)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%f %f", -0.0, 0.0);

	ASSERT(std::strcmp(buffer, "-0. 0.") == 0);
}

TEST_CASE(VFormat_FormatFloat_Exponents)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%f %f %f %f", 9999999999.0, 10000000000., 0.000011, 0.00001);

	ASSERT(std::strcmp(buffer, "9999999999. 1.e10 0.000011 1.e-5") == 0);
}

TEST_CASE(VFormat_FormatFloat_Rounding)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%f %f", 0.9999994999, 0.9999995000);

	ASSERT(std::strcmp(buffer, "0.999999 1.") == 0);
}

TEST_CASE(VFormat_FormatFloat_Range)
{
	char buffer[1024];

	RLM3_Format(buffer, sizeof(buffer), "%f %f %f %f", 4.9406564584124654e-324, 2.2250738585072009e-308, 2.2250738585072014e-308, 1.7976931348623157e308);

	ASSERT(std::strcmp(buffer, "0.e-324 2.225074e-308 2.225074e-308 1.797693e308") == 0);
}
