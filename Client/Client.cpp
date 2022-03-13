// Client.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Client.h"
#include <WS2tcpip.h>

void Client::Run()
{
	Client::ConnectToServer();



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

