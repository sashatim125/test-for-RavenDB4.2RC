#pragma once

extern "C"
{
	__declspec(dllexport) void run_tests(const char* urls[], size_t num_of_urls, const char* db_name, char* result_buffer);
}
