// Server.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Server.h"
#include <time.h>
#include <stdio.h>

void Server::Run()
{
	InitializeFile();


	uint16_t port;
	while (true)
	{
		port = static_cast<uint16_t>(ReadInteger("Enter a port: 0-99999", 0, 99999));

		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (listenSocket == SOCKET_ERROR)
		{
			User::Clear();
			std::cout << "INIT SETUP ERROR: SERVER" << std::endl;
			return;
		}

		sockaddr_in sadd = {};
		sadd.sin_family = AF_INET;
		sadd.sin_port = htons(port);
		sadd.sin_addr.s_addr = INADDR_ANY;

		int result = bind(listenSocket, (SOCKADDR*)&sadd, sizeof(sadd));
		if (result == SOCKET_ERROR)
		{
			User::Clear();
			std::cout << "BIND ERROR: SERVER" << std::endl;
			return;
		}

		result = listen(listenSocket, 1);
		if (result == SOCKET_ERROR)
		{
			User::Clear();
			std::cout << "LISTEN SETUP ERROR: SERVER" << std::endl;
		}
		break;
	}

	std::cout << "Server opened on port " << port << "."<< std::endl;
	AddToFile((char*)[&]() -> std::string {std::string s; s += "Server opened on port " + std::to_string(port) + ".\n"; return s; }().c_str());
	FD_ZERO(&master);
	FD_ZERO(&read);
	FD_ZERO(&write);

	FD_SET(listenSocket, &master);

	for (;;)
	{
		FD_ZERO(&read);
		read = master;

		timeval timer = {};
		timer.tv_sec = NULL;
		
		int numSockets = select(0, &read, nullptr, nullptr, &timer);

		for (size_t i = 0; i < numSockets; i++)
		{
			SOCKET sock = read.fd_array[i];
			if (sock == listenSocket)
			{
				SOCKET s = accept(listenSocket, nullptr, NULL);
				/*if (s == INVALID_SOCKET)
				{
					if (WSAGetLastError() == WSAESHUTDOWN)
					{
						User::Clear();
						std::cout << "SHUTDOWN: SERVER" << std::endl;
						return;
					}
					User::Clear();
					std::cout << "ACCEPT SETUP ERROR: SERVER" << std::endl;
					return;
				}*/
				FD_SET(s, &master);
				acceptedSockets[s] = {nullptr};
				std::cout << "Socket " << s << " connected." << std::endl;
				AddToFile((char*)[&]() -> std::string {std::string str = "Socket "; str += s; str+= " connected.\n"; return str; }().c_str()); // TODO

			}
			else
			{

				char* message = new char[256];
				//readMessage(read.fd_array[i], message);

				uint8_t messageType;

				int bytesread = recv(sock, (char*)&messageType, 1, 0);
				if (bytesread == SOCKET_ERROR || bytesread == 0)
				{
					std::cout << "Socket " << sock << " disconnected." << std::endl;
					//AddToFile((char*)[&]() -> std::string {std::string s = "Socket " + s + " disconnected.\n"; return s; }().c_str()); // TODO
					RemoveUser(sock);
					continue;
				}

				switch (messageType)
				{
				default:
				case static_cast<uint8_t>(MessageTypes::MESSAGE):
				{
					//if (!UserRegistered(sock))
					//{
					//	send(sock, "(You are not registered. Type $register\0", 40, 0);
					//	continue;
					//}
					//GET MESSAGE LENGTH
					uint8_t messageLength;
					recv(sock, (char*)&messageLength, 1, 0);
					

					//READ ENTIRE MESSAGE
					bytesread = recv(sock, message, messageLength + 1, 0);
					message[messageLength] = '\0';

					//DISPLAY MESSAGE
					std::cout << /*GetName(sock)*/ "Socket " << sock << ": " << message << std::endl;
					//AddToFile((char*)[&]() -> std::string {std::string s = GetName(sock); s += ": "; s+= message; return s; }().c_str()); // TODO

					if (bytesread-1 != messageLength)
					{
						std::cout << "PARAMETER ERROR" << std::endl;
						continue;
					}

					//ECHO TO ALL OTHER CLIENTS
					for (size_t s = 0; s < master.fd_count; s++)
					{
						SOCKET us = master.fd_array[s];
						if (us != listenSocket && us != sock)
						{
							char* outmessage = new char[bytesread + 2];
							outmessage[0] = static_cast<char>(messageLength);
							memcpy(&outmessage[1], message, messageLength);
							outmessage[messageLength + 1] = '\0';
							send(us, outmessage, messageLength + 2, 0);
							delete[] outmessage;
						}
					}
					break;
				}
				case static_cast<uint8_t>(MessageTypes::COMMAND):
				{
					uint8_t commandType, messageLength;
					int bytesread = recv(sock, (char*)&commandType, 1, 0);
					bytesread = recv(sock, (char*)&messageLength, 1, 0);



					break;
				}
				}
				delete[] message;
			}
		}


	}

	/*SOCKET s = accept(listenSocket, nullptr, NULL);
	if (s == INVALID_SOCKET)
	{
		if (WSAGetLastError() == WSAESHUTDOWN)
		{
			User::Clear();
			std::cout << "SHUTDOWN: SERVER" << std::endl;
			return;
		}
		User::Clear();
		std::cout << "ACCEPT SETUP ERROR: SERVER" << std::endl;
		return;
	}
	acceptedSockets.push_back(s);
	std::cout << "SOCKET CONNECTED" << std::endl;*/



	Server::Stop();
}

void Server::Stop()
{
	shutdown(listenSocket, SD_BOTH);
	closesocket(listenSocket);

	for (auto& user : acceptedSockets)
	{
		shutdown(user.first, SD_BOTH);
		closesocket(user.first);
	}
}

User* GenerateUser()
{
	return new Server();
}

void Server::readMessage(SOCKET sock, char*& buff)
{
	std::string test;
	test.resize(16);
	int8_t header[2];
	int result = recv(sock, (char*)header, 2, 0);

	if (header[0] == 100)
	{
		buff = new char[header[1] + 1];
		int result = recv(sock, buff, header[1] + 1, 0);
		if (result <= 0)
		{
			std::cout << "Socket " << sock << " disconnected" << std::endl;
			AddToFile((char*)[&]() -> std::string {std::string s = "Socket " + s + " disconnected.\n"; return s; }().c_str()); // TODO
			FD_CLR(sock, &master);
			RemoveUser(sock);
			return;
		}

		return;
	}

	result = recv(sock, (char*)&test, sizeof(test), 0);
	if (result <= 0)
	{
		std::cout << "Socket " << sock << " disconnected" << std::endl;
		AddToFile((char*)[&]() -> std::string {std::string s = "Socket " + s + " disconnected.\n"; return s; }().c_str()); // TODO
		FD_CLR(sock, &master);
		RemoveUser(sock);
		return;
	}
	std::cout << test << std::endl;
}

void Server::RemoveUser(SOCKET sock)
{
	for (std::map<SOCKET, UserData>::iterator iter = acceptedSockets.begin(); iter != acceptedSockets.end(); iter++)
	{
		if (iter->first == sock)
		{
			shutdown(iter->first, SD_BOTH);
			closesocket(iter->first);
			FD_CLR(iter->first, &master);
			acceptedSockets.erase(iter);
			return;
		}
	}
	std::cout << "CRITICAL ERROR: SOCKET ERASED BUT DID NOT EXIST";
	AddToFile((char*)"CRITICAL ERROR: SOCKET ERASED BUT DID NOT EXIST");
}

void Server::echoMessage(char* buff)
{
	if (buff[0] == '0' && buff[1] == '\0')
	{
		return;
	}
	for (size_t i = 0; i < master.fd_count; i++)
	{
		if (!FD_ISSET(master.fd_array[i], &read) && master.fd_array[i] != listenSocket)
		{
			char size = static_cast<char>(sizeofString(buff, 256));

			char* message = new char[size + 1];

			message[0] = size;

			for (size_t i = 0; i < static_cast<int>(size); i++)
			{
				message[i + 1] = buff[i];
			}
			int result = send(master.fd_array[i], message, size + 1, 0);
			if (result == SOCKET_ERROR || result <= 0)
			{
				int t = 0;
			}


		}
	}
}

char* Server::GetName(SOCKET s)
{
	return acceptedSockets[s].username;
}

bool Server::UserRegistered(SOCKET s)
{
	return acceptedSockets[s].username != nullptr;
}

void Server::AddToFile(char* buff)
{
	//FILE* pFile;
	//errno_t err = fopen_s(&pFile, fileName, "a");
	//if (err == 0 && pFile)
	//{
	//	fputs(buff, pFile);
	//	if (pFile) fclose(pFile);
	//}
}

void Server::InitializeFile()
{
	fileName = (char*)"log_test.txt";
	FILE* pFile;
	errno_t err = fopen_s(&pFile, fileName, "w");
	if (pFile) fclose(pFile);
	int t = 0;
	//fileName = (const char*)("log_" + "t");
}