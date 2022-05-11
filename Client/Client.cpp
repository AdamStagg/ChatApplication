// Client.cpp : Defines the exported functions for the DLL.
//
#include "framework.h"
#include "Client.h"
#include <WS2tcpip.h>
#include <thread>



void Client::Run()
{
	Client::ConnectToServer();

	//Start a thread for receiving messages async
	bool* stopThreadFlag = new bool(false);
	std::thread(&Client::receiveMessage, this, stopThreadFlag).detach();

	//Main loop
	while (true)
	{
		//Get user input
		char* userInput = new char[256];
		std::cin.clear();
		std::cout << username << ": ";
		std::cin.getline(userInput, 256);

		//Check if its a command or a message
		if (userInput && userInput[0] == '$')
		{
			sendCommand(sock, userInput, sizeofString(userInput, 256) - 1);
		}
		else
		{
			sendMessage(sock, userInput, sizeofString(userInput, 256) - 1);
		}

		//Check for the exit
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
		//Initialize variables
		username = ReadString("Enter a username:");
		std::string ipaddress = ReadString("Enter an IP address:");
		uint16_t port = static_cast<uint16_t>(ReadInteger("Enter a port number: (0 - 99999)", 0, 99999));

		//Create socket
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == SOCKET_ERROR)
		{
			User::Clear();
			std::cout << "SETUP ERROR: CLIENT" << std::endl;
			continue;
		}

		//Destination setup
		sockaddr_in sadd = {};
		sadd.sin_family = AF_INET;
		inet_pton(AF_INET, ipaddress.c_str(), &sadd.sin_addr.s_addr);
		sadd.sin_port = htons(port);

		//Error check
		if (sadd.sin_addr.s_addr == INADDR_NONE)
		{
			User::Clear();
			std::cout << "ADDRESS ERROR: CLIENT" << std::endl;
			continue;
		}

		//Connect to server
		int result = connect(sock, (SOCKADDR*)&sadd, sizeof(sadd));
		//Error check
		if (result == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAESHUTDOWN)
			{
				std::cout << "SHUTDOWN: CLIENT" << std::endl;
				continue;
			}
			std::cout << "CONNECT ERROR: CLIENT" << std::endl;
			continue;
		}

		//Connected successfully, break infinite loop
		std::cout << "CLIENT SUCCESSFULLY CONNECTED" << std::endl;
		break;
	}

	//Ignore the cin buffer
	std::cin.ignore(INT_MAX, '\n');
}

void Client::Stop()
{
	shutdown(sock, SD_BOTH);
	closesocket(sock);
}

User* GenerateUser()
{
	//Return a new Client for the main function
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

	arr[0] = m; //Message
	arr[1] = len; //Message size
	std::memcpy(&arr[2], buff, length); //copy message
	arr[length + 2] = '\0'; //null terminator

	/*for (size_t i = 0; i < length; i++)
	{
		arr[i + 2] = buff[i];
	}*/
	//message += (char*)&length;
	//message += buff;
	int bytessent = send(sock, arr, length + 3, 0); //send the message
	if (bytessent <= 0) //error check
	{
		CloseServer();
	}
	delete[] arr;
}

void Client::sendCommand(SOCKET sock, char* buff, const int32_t length)
{
	char* message;
	char commandType; //predefine, incorperate later

	//Check for the register command
	if (strstr(tolower(buff, length), "$register")) 
	{
		//REGISTER BYTE FORMAT::
		//First byte = command byte
		//Second byte = register command type
		//Third byte = username length
		//Rest of the message = username
		char len = static_cast<char>(username.size() + 4); 
		message = new char[len];
		commandType = static_cast<char>(CommandTypes::REGISTER);
		message[2] = static_cast<char>(username.size() + 1);
		memcpy(&message[3], username.c_str(), username.size() + 1);
	}
	else
	{
		//OTHER COMMAND BYTE FORMAT::
		//First byte = command byte
		//Second byte = command type
		//Third byte = null terminator
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

	if (commandType == 0) //Command not found
	{
		std::cout << "Command not recognized. Please try again\n";
		delete[] message;
		return;
	}

	//All commands have these two bytes
	message[0] = static_cast<char>(MessageTypes::COMMAND);
	message[1] = commandType;

	int bytessent = send(sock, message, sizeofString(message, 256), 0); //send the command
	if (bytessent <= 0) //error check
	{
		CloseServer();
	}
	delete[] message;
}

void Client::receiveEcho(SOCKET sock, char*& buff)
{
	int8_t type;
	int result = recv(sock,(char*) & type, 1, 0); //recv into type
	if (result <= 0) //error check
	{
		CloseServer();
		return;
	}

	if (type == static_cast<int>(SendTypes::SINGLE)) //single message, not message stream
	{

		int8_t size;
		int result = recv(sock, (char*)&size, 1, 0); //recv into size
		if (result <= 0) // error check
		{
			CloseServer();
			return;
		}

		//Initialize buffer with size
		buff = new char[size + 1];
		result = recv(sock, buff, size + 1, 0); //Recv into the buffer
		if (result <= 0) //Error check
		{
			CloseServer();
			return;
		}
	}
	else // Message stream, not a single message
	{
		//Make the buffer
		char line[256];
		line[255] = '\0';

		//Open the file
		FILE* f;
		std::ostringstream stream;
		stream << "../" << "log_client.txt";
		fopen_s(&f, stream.str().c_str(), "w");
		if (f) //If the file is open
		{
			while ((strstr(line, "##ENDFILE") == nullptr)) //Loop until the end of the file
			{

				int bytesread = recv(sock, line, sizeof(line) - 1, 0); //Recv the current message
				if (bytesread <= 0) //Error check
				{
					CloseServer();
				}
				fputs(line, f); //Write to the file
			}


			buff = new char[18];
			memcpy(buff, "The log has been received.\n\0", 18);
			fclose(f);
		}
	}
}

void Client::receiveMessage(bool* stopFlag)
{
	//Create a set to multiplex
	FD_SET set;
	FD_ZERO(&set);
	FD_SET(sock, &set);

	while (!(* stopFlag))
	{
		char* message;

		if (select(0, &set, nullptr, nullptr, nullptr)) //Determine if socket has activity
		{
			receiveEcho(sock, message); //Receive the message as dynamic memory
			if (!serverRunning) 
			{
				(*stopFlag) = true;
				break;
			}
			if (message != nullptr)
			{
				//std::cout << "echo received" << std::endl;
				//Write the echo to the screen
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