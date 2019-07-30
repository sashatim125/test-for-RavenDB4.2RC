
#include "pch.h"
#include "Tester.h"
#include <iostream>


extern "C"
{
	void __declspec(dllexport) _stdcall
		run_tests(
			const char* urls[],
			int32_t num_of_urls,
			const char* db_name,
			void* result_buffer,
			int32_t* result_buffer_length)
	{
		auto tester = test::Tester(urls, num_of_urls, db_name);
		tester.run();
		tester.fillResultBuffer((char*)result_buffer, result_buffer_length);
	}
}


//int main()
//{
//	const char* urls[] = { "http://127.0.0.1:8081","http://127.0.0.1:8080" };
//	auto buffer = new char[1024 * 1024];
//	run(urls,2,"Test", buffer);
//	std::cout << buffer;
//	delete[] buffer;
//}
