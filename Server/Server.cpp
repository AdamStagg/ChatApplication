// Server.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Server.h"

void Server::Run()
{
	InitializeFile();
	InitializeServer();

	oss << "Server opened on port " << port << "." << std::endl;
	PrintAndWrite(oss.str().c_str());

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
				AddUser();
			}
			else
			{
				readMessage(sock);
			}
		}
	}
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

void Server::readMessage(SOCKET sock)
{
	char* message = new char[256];
	uint8_t messageType;

	int bytesread = recv(sock, (char*)&messageType, 1, 0);
	if (bytesread == SOCKET_ERROR || bytesread == 0)
	{
		oss << "Socket " << sock << " disconnected." << std::endl;
		//std::cout << "Socket " << sock << " disconnected." << std::endl;
		PrintAndWrite(oss.str().c_str());
		RemoveUser(sock);
		return;
	}

	switch (messageType)
	{
	default:
	case static_cast<uint8_t>(MessageTypes::MESSAGE):
	{
		if (!UserRegistered(sock))
		{
			send(sock, "(You are not registered. Type $register\0", 40, 0);
			char t[256];
			recv(sock, t, 256, 0);
			return;
		}
		//GET MESSAGE LENGTH
		uint8_t messageLength;
		recv(sock, (char*)&messageLength, 1, 0);


		//READ ENTIRE MESSAGE
		bytesread = recv(sock, message, messageLength + 1, 0);
		message[messageLength] = '\0';

		//DISPLAY MESSAGE
		oss << ((GetName(sock) != nullptr) ? GetName(sock) : ("Socket " + sock)) << ": " << message << std::endl;
		PrintAndWrite(oss.str().c_str());
		//AddToFile((char*)[&]() -> std::string {std::string s = GetName(sock); s += ": "; s+= message; return s; }().c_str()); // TODO

		if (bytesread - 1 != messageLength)
		{
			std::cout << "PARAMETER ERROR" << std::endl;
			return;
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
				int bytessend = send(us, outmessage, messageLength + 2, 0);
				if (bytessend <= 0)
				{
					RemoveUser(us);
				}
				delete[] outmessage;
			}
		}
		break;
	}
	case static_cast<uint8_t>(MessageTypes::COMMAND):
	{
		uint8_t commandType;
		int bytesread = recv(sock, (char*)&commandType, 1, 0);
		if (bytesread <= 0) RemoveUser(sock);
		std::string list = " ";
		switch (commandType)
		{
		case static_cast<uint8_t>(User::CommandTypes::REGISTER):
			int8_t userNameLength;
			bytesread = recv(sock, (char*)&userNameLength, 1, 0);

			acceptedSockets[sock] = new char[userNameLength + 1];

			bytesread = recv(sock, acceptedSockets[sock], userNameLength + 1, 0);

			oss << "Socket " << std::to_string(sock) << " has been registered to the username " << acceptedSockets[sock] << std::endl;
			PrintAndWrite(oss.str().c_str());

			break;
		case static_cast<uint8_t>(User::CommandTypes::GETLOG):
			break;
		case static_cast<uint8_t>(User::CommandTypes::GETLIST):
			for (auto& user : acceptedSockets)
			{
				if (user.second == nullptr)
				{
					list += "Socket ";
					list += std::to_string(user.first);
					list += ", ";
				}
				else
				{
					list += user.second;
					list += ", ";
				}
			}
			list = list.substr(0, list.size() - 2);
			list += '\0';
			list[0] = static_cast<char>(list.size() + 2);
			send(sock, list.c_str(), list.size() + 1, 0);
			list = list.substr(1, list.size());

			oss << "Sending user " << acceptedSockets[sock] << " the user list:" << std::endl << list << std::endl;
			PrintAndWrite(oss.str().c_str());


			break;
		case static_cast<uint8_t>(User::CommandTypes::COUNT):
		default:
		case static_cast<uint8_t>(User::CommandTypes::EXIT):
			RemoveUser(sock);
			break;
		}

		//bytesread = recv(sock, (char*)&messageLength, 1, 0);



		break;
	}
	}
	delete[] message;
}

void Server::AddUser()
{
	SOCKET s = accept(listenSocket, nullptr, NULL);
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
	if (maxClients <= acceptedSockets.size())
	{
		oss << "Socket " << std::to_string(s) << " tried to connect, but the server was full." << std::endl;
		PrintAndWrite(oss.str().c_str());
		char m[20];
		memcpy(m, " The server is full\0", 20);
		m[0] = static_cast<char>(20);
		send(s, m, 20, 0);
		shutdown(s, SD_BOTH);
		closesocket(s);
		return;
	}
	FD_SET(s, &master);
	acceptedSockets[s] = nullptr;
	oss << "Socket " << std::to_string(s) << " connected." << std::endl;
	PrintAndWrite(oss.str().c_str());
	//std::cout << "Socket " << s << " connected." << std::endl;
	//AddToFile((char*)[&]() -> std::string {std::string str = "Socket "; str += (char*)&s; str += " connected.\n"; return str; }().c_str()); // TODO
}

void Server::RemoveUser(SOCKET sock)
{
	for (std::map<SOCKET, char*>::iterator iter = acceptedSockets.begin(); iter != acceptedSockets.end(); iter++)
	{
		if (iter->first == sock)
		{
			oss << "Socket " << sock << " disconnected." << std::endl;
			PrintAndWrite(oss.str().c_str());
			shutdown(iter->first, SD_BOTH);
			closesocket(iter->first);
			FD_CLR(iter->first, &master);
			acceptedSockets.erase(iter);
			return;
		}
	}
	PrintAndWrite("CRITICAL ERROR: USER NOT FOUND BUT WAS REMOVED");
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
	return acceptedSockets[s];
}

bool Server::UserRegistered(SOCKET s)
{
	return acceptedSockets[s] != nullptr;
}

void Server::AddToFile(const char* buff)
{
	FILE* pFile;
	errno_t err = fopen_s(&pFile, fileName, "a");
	if (err == 0 && pFile)
	{
		fputs(buff, pFile);
		fclose(pFile);
	}
}

void Server::InitializeFile()
{
	fileName = (char*)"../Server/log_test.txt";
	FILE* pFile;
	errno_t err = fopen_s(&pFile, fileName, "w");
	if (pFile) fclose(pFile);
	int t = 0;
	//fileName = (const char*)("log_" + "t");
}

void Server::InitializeServer()
{
	while (true)
	{
		maxClients = static_cast<uint8_t>(ReadInteger("Max number of clients: (0-99)", 0, 99));
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

	FD_ZERO(&master);
	FD_ZERO(&read);

	FD_SET(listenSocket, &master);
}

void Server::PrintAndWrite(const char* buff)
{
	std::cout << buff;
	AddToFile(buff);
	oss = std::ostringstream();
}