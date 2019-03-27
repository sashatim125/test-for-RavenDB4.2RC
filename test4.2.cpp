
#include "pch.h"
#include "test4.2.h"
#include "Tester.h"



	void run_tests(const char* urls[], size_t num_of_urls, const char* db_name, char* result_buffer)
	{
		auto tester = test::Tester(urls, num_of_urls, db_name);
		tester.run();
		tester.fillResultBuffer(result_buffer);
	}


//int main()
//{
//	const char* urls[] = { "http://127.0.0.1:8081","http://127.0.0.1:8080" };
//	auto buffer = new char[1024 * 1024];
//	run(urls,2,"Test", buffer);
//	std::cout << buffer;
//	delete[] buffer;
//}
