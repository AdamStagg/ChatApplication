// Server.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Server.h"

void Server::Run()
{
	while (true)
	{
		uint16_t port = static_cast<uint16_t>(ReadInteger("Enter a port: 0-99999", 0, 99999));

		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (listenSocket == SOCKET_ERROR)
		{
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
			std::cout << "BIND ERROR: SERVER" << std::endl;
			return;
		}

		result = listen(listenSocket, 1);
		if (result == SOCKET_ERROR)
		{
			std::cout << "LISTEN SETUP ERROR: SERVER" << std::endl;
		}

		SOCKET s = accept(listenSocket, nullptr, NULL);
		if (s == INVALID_SOCKET)
		{
			if (WSAGetLastError() == WSAESHUTDOWN)
			{
				std::cout << "SHUTDOWN: SERVER" << std::endl;
				return;
			}
			std::cout << "ACCEPT SETUP ERROR: SERVER" << std::endl;
			return;
		}
		acceptedSockets.push_back(s);
		std::cout << "SOCKET CONNECTED" << std::endl;

	}

	Server::Stop();
}

void Server::Stop()
{
	shutdown(listenSocket, SD_BOTH);
	closesocket(listenSocket);

	for (auto& sock : acceptedSockets)
	{
		shutdown(sock, SD_BOTH);
		closesocket(sock);
	}
}

User* GenerateUser()
{
	return new Server();
}