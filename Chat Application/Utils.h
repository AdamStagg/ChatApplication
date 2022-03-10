#pragma once
#include <iostream>

int ReadInteger(const char* prompt)
{
	std::cout << prompt << std::endl;

	int result;
	
	std::cin >> result;

	while (std::cin.fail())
	{
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
		std::cout << prompt << std::endl;
		std::cin >> result;
	}

	return result;

}