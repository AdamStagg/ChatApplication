// Client.cpp : Defines the exported functions for the DLL.
//
#include "framework.h"
#include "Client.h"
#include <WS2tcpip.h>
#include <thread>



void Client::Run()
{
	Client::ConnectToServer();

	bool* stopThreadFlag = new bool(false);
	std::thread(&Client::receiveMessage, this, stopThreadFlag).detach();

	while (true)
	{
		char* userInput = new char[256];
		std::cin.clear();
		std::cout << username << ": ";
		std::cin.getline(userInput, 256);
		if (userInput && userInput[0] == '$')
		{
			sendCommand(sock, userInput, sizeofString(userInput, 256) - 1);
		}
		else
		{
			sendMessage(sock, userInput, sizeofString(userInput, 256) - 1);
		}
		if (strstr(userInput, "$exit") != nullptr || !serverRunning) { break; }
		delete[] userInput;
	}

	(*stopThreadFlag) = true;
	

	Client::Stop();
}

void Client::ConnectToServer()
{
	while (true)
	{
		username = ReadString("Enter a username:");
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

	std::cin.ignore(INT_MAX, '\n');
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
	int bytessent = send(sock, arr, length + 3, 0);
	if (bytessent <= 0)
	{
		CloseServer();
	}
	delete[] arr;
}

void Client::sendCommand(SOCKET sock, char* buff, const int32_t length)
{
	char* message;
	
	char commandType;
	if (strstr(tolower(buff, length), "$register"))
	{
		char len = static_cast<char>(username.size() + 4);
		message = new char[len];
		commandType = static_cast<char>(CommandTypes::REGISTER);
		message[2] = static_cast<char>(username.size() + 1);
		memcpy(&message[3], username.c_str(), username.size() + 1);
	}
	else
	{
		message = new char[3];
		if (strstr(tolower(buff, length), "$exit"))
			commandType = static_cast<char>(CommandTypes::EXIT);
		else if (strstr(tolower(buff, length), "$getlog"))
			commandType = static_cast<char>(CommandTypes::GETLOG);
		else if (strstr(tolower(buff, length), "$getlist"))
			commandType = static_cast<char>(CommandTypes::GETLIST);
		else commandType = 0;
		message[2] = '\0';
	}

	message[0] = static_cast<char>(MessageTypes::COMMAND);
	message[1] = commandType;

	int bytessent = send(sock, message, sizeofString(message, 256), 0);
	if (bytessent <= 0)
	{
		CloseServer();
	}
	delete[] message;
}

void Client::receiveEcho(SOCKET sock, char*& buff)
{
	int8_t type;
	int result = recv(sock,(char*) & type, 1, 0);
	if (type == static_cast<int>(SendTypes::SINGLE))
	{

		int8_t size;
		int result = recv(sock, (char*)&size, 1, 0);
		if (result <= 0)
		{
			CloseServer();
			return;
		}
		buff = new char[size + 1];
		result = recv(sock, buff, size + 1, 0);
		if (result <= 0)
		{
			CloseServer();
			return;
		}
	}
	else
	{
		char line[256];
		line[255] = '\0';
		FILE* f;
		std::ostringstream stream;
		stream << "../" << "log_client.txt";
		fopen_s(&f, stream.str().c_str(), "w");
		if (f)
		{
			while ((strstr(line, "##ENDFILE") == nullptr))
			{

				int bytesread = recv(sock, line, sizeof(line) - 1, 0);
				if (bytesread <= 0)
				{
					CloseServer();
				}
				//if (strstr(line, "##ENDFILE") != nullptr) break;
				fputs(line, f);
			}
			buff = new char[sizeof("The log has been received.")+2];
			memcpy(buff, "The log has been received.\n\0", sizeof("The log has been received.\n\0"));
			fclose(f);
		}
	}
}

void Client::receiveMessage(bool* stopFlag)
{
	FD_SET set;
	FD_ZERO(&set);
	FD_SET(sock, &set);
	while (true)
	{
		if ((*stopFlag) == true) break;
		char* message;

		if (select(0, &set, nullptr, nullptr, nullptr))
		{
			receiveEcho(sock, message);
			if (!serverRunning) 
			{
				(*stopFlag) = true;
				break;
			}
			if (message != nullptr)
			{
				//std::cout << "echo received" << std::endl;
				std::cout << std::endl << message << std::endl << username << ": ";
			}
		}
	}
	delete stopFlag;
}

void Client::CloseServer()
{
	shutdown(sock, SD_BOTH);
	closesocket(sock);
	serverRunning = false;
	std::cout << std::endl << "The server has been closed." << std::endl;
}