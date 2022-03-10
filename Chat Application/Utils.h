#pragma once
#include <iostream>
#include <string>

/// <summary>
/// A function that displays a prompt and asks the user for inputs
/// </summary>
/// <param name="prompt">A null terminated string to display</param>
/// <param name="min">Inclusive min</param>
/// <param name="max">Inclusive max</param>
/// <returns>An integer for the user's input</returns>
int ReadInteger(const char* prompt, const int min, const int max)
{
	std::cout << prompt << std::endl;

	int result;
	
	std::cin >> result;

	while (std::cin.fail() || result < min || result > max)
	{
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
		std::cout << prompt << std::endl;
		std::cin >> result;
	}

	return result;
}

/// <summary>
/// A function that displays a prompt and asks the user for a string
/// </summary>
/// <param name="prompt">The prompt to display</param>
/// <returns>A string for the user's input</returns>
std::string ReadString(const char* prompt)
{
	std::cout << prompt << std::endl;

	std::string result;

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