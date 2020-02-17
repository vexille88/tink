#include <iostream>
#include <vector>
#include <string>
#include <ctime>

#define allocate_length 100000

int main()
{
	std::clock_t start = 0, end = 0;
	start = std::clock();
	std::string *str1 = new std::string[allocate_length];
	auto str6 = str1;
	for(int i = 0; i < allocate_length; i++){
		*str1++ = "Hello World!";
	}
	delete []str6;
	end = std::clock();
	std::cout << (double(end - start) / CLOCKS_PER_SEC) << std::endl;

}
