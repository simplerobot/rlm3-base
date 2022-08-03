#include "Test.hpp"
#include "rlm3-bytes.h"
#include <cstring>
#include <cmath>


TEST_CASE(hton_u16_HappyCase)
{
	uint16_t host = 0x1234;

	uint16_t network = hton_u16(host);

	uint8_t* network_bytes = (uint8_t*)&network;
	uint8_t expected_bytes[] = { 0x12, 0x34 };
	for (size_t i = 0; i < sizeof(network); i++)
		ASSERT(network_bytes[i] == expected_bytes[i]);
}

TEST_CASE(hton_i16_HappyCase)
{
	int16_t host = 0x1234;

	int16_t network = hton_i16(host);

	uint8_t* network_bytes = (uint8_t*)&network;
	uint8_t expected_bytes[] = { 0x12, 0x34 };
	for (size_t i = 0; i < sizeof(network); i++)
		ASSERT(network_bytes[i] == expected_bytes[i]);
}

TEST_CASE(hton_u32_HappyCase)
{
	uint32_t host = 0x12345678;

	uint32_t network = hton_u32(host);

	uint8_t* network_bytes = (uint8_t*)&network;
	uint8_t expected_bytes[] = { 0x12, 0x34, 0x56, 0x78 };
	for (size_t i = 0; i < sizeof(network); i++)
		ASSERT(network_bytes[i] == expected_bytes[i]);
}

TEST_CASE(hton_i32_HappyCase)
{
	int32_t host = 0x12345678;

	int32_t network = hton_i32(host);

	uint8_t* network_bytes = (uint8_t*)&network;
	uint8_t expected_bytes[] = { 0x12, 0x34, 0x56, 0x78 };
	for (size_t i = 0; i < sizeof(network); i++)
		ASSERT(network_bytes[i] == expected_bytes[i]);
}

TEST_CASE(hton_f32_HappyCase)
{
	float host = -0.0;

	float network = hton_f32(host);

	uint8_t* network_bytes = (uint8_t*)&network;
	uint8_t expected_bytes[] = { 0x80, 0x00, 0x00, 0x00 };
	for (size_t i = 0; i < sizeof(network); i++)
		ASSERT(network_bytes[i] == expected_bytes[i]);
}

TEST_CASE(hton_f64_HappyCase)
{
	double host = -0.0;

	double network = hton_f64(host);

	uint8_t* network_bytes = (uint8_t*)&network;
	uint8_t expected_bytes[] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	for (size_t i = 0; i < sizeof(network); i++)
		ASSERT(network_bytes[i] == expected_bytes[i]);
}

TEST_CASE(ntoh_u16_HappyCase)
{
	uint8_t network_bytes[] = { 0x12, 0x34 };
	uint16_t network = 0;
	std::memcpy(&network, network_bytes, sizeof(network));

	uint16_t host = ntoh_u16(network);

	ASSERT(host == 0x1234);
}

TEST_CASE(ntoh_i16_HappyCase)
{
	uint8_t network_bytes[] = { 0x12, 0x34 };
	int16_t network = 0;
	std::memcpy(&network, network_bytes, sizeof(network));

	int16_t host = ntoh_i16(network);

	ASSERT(host == 0x1234);
}

TEST_CASE(ntoh_u32_HappyCase)
{
	uint8_t network_bytes[] = { 0x12, 0x34, 0x56, 0x78 };
	uint32_t network = 0;
	std::memcpy(&network, network_bytes, sizeof(network));

	uint32_t host = ntoh_u32(network);

	ASSERT(host == 0x12345678);
}

TEST_CASE(ntoh_i32_HappyCase)
{
	uint8_t network_bytes[] = { 0x12, 0x34, 0x56, 0x78 };
	int32_t network = 0x12345678;
	std::memcpy(&network, network_bytes, sizeof(network));

	int32_t host = ntoh_i32(network);

	ASSERT(host == 0x12345678);
}

TEST_CASE(ntoh_f32_HappyCase)
{
	uint8_t network_bytes[] = { 0x80, 0x00, 0x00, 0x00 };
	float network = 0.0;
	std::memcpy(&network, network_bytes, sizeof(network));

	float host = ntoh_f32(network);

	ASSERT(std::signbit(host) == true);
	ASSERT(host == -0.0);
}

TEST_CASE(ntoh_f64_HappyCase)
{
	uint8_t network_bytes[] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	double network = -0.0;
	std::memcpy(&network, network_bytes, sizeof(network));

	double host = ntoh_f64(network);

	ASSERT(std::signbit(host) == true);
	ASSERT(host == -0.0);
}


