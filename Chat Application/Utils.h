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
int ReadInteger(const char* prompt, const int min = -INT_MAX, const int max = INT_MAX)
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

/// <summary>
/// Gets the size of a string stored in a char*
/// </summary>
/// <param name="buffer">A null terminated string to check the length of</param>
/// <param name="maxLength">The size of the array</param>
/// <returns>The size of the string</returns>
int sizeofString(char* buffer, int maxLength)
{
	int counter = 0;
	for (size_t i = 0; i < maxLength; i++)
	{
		counter++;
		if (buffer[i] == '\0')
		{
			return counter;
		}
	}
	return maxLength;
}

/// <summary>
/// A function that converts a string to lowercase
/// </summary>
/// <param name="buff">The string</param>
/// <param name="size">The size of the array</param>
/// <returns></returns>
char* tolower(const char* buff, int size)
{
	char* word = (char*)buff;

	for (size_t i = 1; i < size; i++)
	{
		if (word[i] < 97) word[i] = word[i] + 32;
	}

	return word;
}