
#include <chrono>
#include <iostream>
#include <thread>
#include "..\includes\thread_pool.hpp"



void PrintBig1()
{
	for (int i = 0; i < 100; ++i)
	{
		std::cout << "thread id = " << std::this_thread::get_id() << " value = " << i << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

int main() 
{
	using namespace stud_tpl;

	for (size_t i = 0; i < 10; i++)
	{
		thread_pool::spawn([&]() {PrintBig1(); });
	}


	return 1;
}