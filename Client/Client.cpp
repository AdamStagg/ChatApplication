// Client.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Client.h"
#include <WS2tcpip.h>
#include <iostream>
#include <string>

void Client::Run()
{
	Client::ConnectToServer();

	std::cin.ignore(INT_MAX, '\n');
	while (true)
	{
		char* userInput = new char[256];
		std::cin.clear();
		std::cin.getline(userInput, 256);
		if (userInput && userInput[0] == '$')
		{

		}
		else
		{
			sendMessage(sock, userInput, sizeofString(userInput, 256) - 1);
		}
		if (strstr(userInput, "$exit") != nullptr) { break; }
		delete[] userInput;

		char* message;

		FD_SET set;
		FD_ZERO(&set);
		FD_SET(sock, &set);
		timeval timer;
		timer.tv_sec = 0;

		if (select(FD_SETSIZE, &set, nullptr, nullptr, &timer))
		{
			receiveEcho(sock, message);
			if (message != nullptr)
			{
				//std::cout << "echo received" << std::endl;
				std::cout << message << std::endl;
			}
		}
		
	}
	

	Client::Stop();
}

void Client::ConnectToServer()
{
	while (true)
	{
		std::string username = ReadString("Enter a username:");
		std::string ipaddress = ReadString("Enter an IP address:");
		uint16_t port = static_cast<uint16_t>(ReadInteger("Enter a port number: (0 - 99999)", 0, 99999));


		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == SOCKET_ERROR)
		{
			User::Clear();
			std::cout << "SETUP ERROR: CLIENT" << std::endl;
			continue;
		}

		sockaddr_in sadd = {};
		sadd.sin_family = AF_INET;
		char address[16];
		for (size_t i = 0; i < ipaddress.size(); i++)
		{
			address[i] = ipaddress[i];
		}
		address[ipaddress.size()] = '\0';
		inet_pton(AF_INET, ipaddress.c_str(), &sadd.sin_addr.s_addr);
		sadd.sin_port = htons(port);

		if (sadd.sin_addr.s_addr == INADDR_NONE)
		{
			User::Clear();
			std::cout << "ADDRESS ERROR: CLIENT" << std::endl;
			continue;
		}

		int result = connect(sock, (SOCKADDR*)&sadd, sizeof(sadd));
		if (result == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAESHUTDOWN)
			{
				User::Clear();
				std::cout << "SHUTDOWN: CLIENT" << std::endl;
				continue;
			}
			User::Clear();
			std::cout << "CONNECT ERROR: CLIENT" << std::endl;
			continue;
		}

		User::Clear();
		
		std::cout << "CLIENT SUCCESSFULLY CONNECTED" << std::endl;
		break;
	}
}

void Client::Stop()
{
	shutdown(sock, SD_BOTH);
	closesocket(sock);
}

User* GenerateUser()
{
	return new Client();
}
/// <summary>
/// Sends a message to the server
/// 
/// <h>
/// HEADER FORMAT:
/// 32 bits for message type (message = 100, command = 200)
/// 32 bits for size of message
/// 32 bits for size of username (if message type is message)
/// </h>
/// </summary>
void Client::sendMessage(SOCKET sock, char* buff, const int32_t length)
{
	std::string message;
	char m = static_cast<char>(MessageTypes::MESSAGE);
	char len = (char&)length;
	char* arr = new char[length + 3];

	arr[0] = m;
	arr[1] = len;
	for (size_t i = 0; i < length; i++)
	{
		arr[i + 2] = buff[i];
	}
	arr[length + 2] = '\0';
	//message += (char*)&length;
	//message += buff;
	int t = 0;
	send(sock, arr, length + 3, 0);
	delete[] arr;
}

void Client::receiveEcho(SOCKET sock, char*& buff)
{
	int8_t size;
	int result = recv(sock, (char*)&size, 1, 0);
	if (result <= 0)
	{
		std::cout << "The server has been closed." << std::endl;
		return;
	}
	buff = new char[size + 1];
	result = recv(sock, buff, size + 1, 0);
	if (result <= 0)
	{
		std::cout << "The server has been closed." << std::endl;
		return;
	}
}