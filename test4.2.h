#pragma once
#include <cstdint>


void run_tests(
	const char* urls[],
	int32_t num_of_urls,
	const char* db_name,
	void* result_buffer,
	int32_t* result_buffer_length);

