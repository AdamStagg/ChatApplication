// Server.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Server.h"
#include <stdio.h>
#include <stdlib.h>

void Server::Run()
{
	InitializeFile();
	InitializeServer();

	oss << "Server opened on port " << port << "." << std::endl;
	PrintAndWrite(oss.str().c_str());

	for (;;) //Loop until close
	{
		//Copy the master set
		FD_ZERO(&read);
		read = master;

		timeval timer = {};
		timer.tv_sec = NULL;
		
		int numSockets = select(0, &read, nullptr, nullptr, &timer); //Select active sockets

		for (size_t i = 0; i < numSockets; i++) //Loop over active sockets
		{
			SOCKET sock = read.fd_array[i];
			if (sock == listenSocket) //If listening socket add user
			{
				AddUser();
			}
			else //else a message has been received
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

	//loop over all accepted sockets and close them
	for (auto& user : acceptedSockets)
	{
		shutdown(user.first, SD_BOTH);
		closesocket(user.first);
	}
}

User* GenerateUser()
{
	//Return a new object for the main function
	return new Server();
}

//Reads a message from a client
void Server::readMessage(SOCKET sock)
{
	char* message = new char[256];
	uint8_t messageType;

	int bytesread = recv(sock, (char*)&messageType, 1, 0); //recv into messageType
	if (messageType == 0) return; //Error check
	if (bytesread == SOCKET_ERROR || bytesread == 0) //Also error check
	{
		oss << "Socket " << sock << " disconnected." << std::endl;
		//std::cout << "Socket " << sock << " disconnected." << std::endl;
		PrintAndWrite(oss.str().c_str());
		RemoveUser(sock);
		return;
	}

	switch (messageType) //Message or command
	{
	default:
	case static_cast<uint8_t>(MessageTypes::MESSAGE):
	{
		//Checks if registered
		if (!UserRegistered(sock))
		{
			// 2( is for the header, 2 = message type, ( = length of the message as characters
			send(sock, "2(You are not registered. Type $register\0", 41, 0);
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
		std::ostringstream oss2;
		oss2 << static_cast<char>(SendTypes::SINGLE) << static_cast<char>(sizeofString((char*)oss.str().c_str(), 256)) << ((GetName(sock) != nullptr) ? GetName(sock) : ("Socket " + sock)) << ": " << message << std::endl;
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
				int bytessend = send(us, oss2.str().c_str(), oss2.str().size() + 1, 0); //Send to user
				if (bytessend <= 0) //Error check
				{
					RemoveUser(us);
				}
			}
		}
		break;
	}
	case static_cast<uint8_t>(MessageTypes::COMMAND):
	{

		uint8_t commandType;
		int bytesread = recv(sock, (char*)&commandType, 1, 0); //Recv into commandType
		if (bytesread <= 0) RemoveUser(sock); //Error check


		//Variable predeclaration
		FILE* f;
		std::ostringstream s;
		std::string list;
		char lineBuf[256];
		std::string line;
		list += static_cast<char>(SendTypes::SINGLE);
		list += " "; //leave space for the command type
		switch (commandType)
		{
		case static_cast<uint8_t>(User::CommandTypes::REGISTER):
			//Recv length, and username
			//Register user


			int8_t userNameLength;
			bytesread = recv(sock, (char*)&userNameLength, 1, 0);

			acceptedSockets[sock] = new char[userNameLength + 1];

			bytesread = recv(sock, acceptedSockets[sock], userNameLength + 1, 0);

			oss << "Socket " << std::to_string(sock) << " has been registered to the username " << acceptedSockets[sock] << std::endl;
			PrintAndWrite(oss.str().c_str());

			break;
		case static_cast<uint8_t>(User::CommandTypes::GETLOG):

			//Open log file, relay all lines to the client

			fopen_s(&f, fileName, "r");

			if (bytesread <= 0)
			{
				RemoveUser(sock);
			}
			if (f)
			{
				char stype = static_cast<char>(SendTypes::STREAM); //Multiple messages
				bytesread = send(sock, &stype, 1, 0); //Send the type of message
				if (bytesread <= 0) //Error check
				{
					RemoveUser(sock);
				}
				while (fgets(lineBuf, sizeof(lineBuf), f)) //Read the file
				{
					//lineBuf[0] = static_cast<char>(SendTypes::STREAM);
					bytesread = send(sock, lineBuf, sizeof(lineBuf) - 1, 0); //Send the message
					if (bytesread <= 0) //Error check
					{
						RemoveUser(sock);
					}
				}

				bytesread = send(sock, "###ENDFILE\0", sizeof("###ENDFILE\0"), 0); //End of file
				if (bytesread <= 0) //Error check
				{
					RemoveUser(sock);
				}

				//Display success
				oss << "Send the log to user " << GetName(sock) << std::endl;
				PrintAndWrite(oss.str().c_str());
				fclose(f);
			}
			else PrintAndWrite("There was no file found...\n");

			break;
		case static_cast<uint8_t>(User::CommandTypes::GETLIST):

			//Sends a message containing the names of all connected clients

			for (auto& user : acceptedSockets) //Loop over clients
			{
				if (user.second == nullptr) //Checks if they don't have a registered name
				{
					list += "Socket ";
					list += std::to_string(user.first);
				}
				else //They do have a registered name, use that
				{
					list += user.second;
				}
				list += ", "; //All entries need a comma
			}

			//Remove the final comma for formatting
			list = list.substr(0, list.size() - 2);
			list[1] = static_cast<char>(list.size() + 2);

			send(sock, list.c_str(), list.size() + 1, 0); //Send the message to the client
			list = list.substr(2, list.size());

			//Log the action
			oss << "Sending user " << acceptedSockets[sock] << " the user list:" << std::endl << list << std::endl;
			PrintAndWrite(oss.str().c_str());
			break;


		//All other cases treat as exit
		case static_cast<uint8_t>(User::CommandTypes::COUNT):
		default:
		case static_cast<uint8_t>(User::CommandTypes::EXIT):
			RemoveUser(sock);
			break;
		}

		break;
	}
	}
	delete[] message;
}

//Adds a user to the server
void Server::AddUser()
{
	SOCKET s = accept(listenSocket, nullptr, NULL);
	if (s == INVALID_SOCKET) //Error check
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

	if (maxClients <= acceptedSockets.size()) //Check if we don't have room
	{
		oss << "Socket " << std::to_string(s) << " tried to connect, but the server was full." << std::endl;
		PrintAndWrite(oss.str().c_str());
		char m[21];
		m[0] = static_cast<char>(SendTypes::SINGLE);
		m[1] = static_cast<char>(21); //Size of message
		memcpy(&m[2], "The server is full\0", 19);
		send(s, m, 21, 0); //Send the message
		//Close the socker, we are full
		shutdown(s, SD_BOTH);
		closesocket(s);
		return;
	}

	//We have room, add the user
	FD_SET(s, &master);
	acceptedSockets[s] = nullptr;
	oss << "Socket " << std::to_string(s) << " connected." << std::endl;
	PrintAndWrite(oss.str().c_str());
}

//Removes a user from the server
void Server::RemoveUser(SOCKET sock)
{
	//Loop over the entire dictionary
	for (std::map<SOCKET, char*>::iterator iter = acceptedSockets.begin(); iter != acceptedSockets.end(); iter++)
	{
		if (iter->first == sock) //If we found a match
		{
			//Disconnect the user
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

//Relays the message to all other clients
void Server::echoMessage(char* buff)
{
	//Error check
	if (buff[0] == '0' && buff[1] == '\0')
	{
		return;
	}

	for (size_t i = 0; i < master.fd_count; i++) //Loop over all clients
	{
		if (!FD_ISSET(master.fd_array[i], &read) && master.fd_array[i] != listenSocket)
		{
			//Send message with format:
			//First byte = size
			//Rest of message
			char size = static_cast<char>(sizeofString(buff, 256));

			char* message = new char[size + 1];

			message[0] = size;

			for (size_t i = 0; i < static_cast<int>(size); i++)
			{
				message[i + 1] = buff[i];
			}
			std::ostringstream stream;
			stream << static_cast<char>(SendTypes::SINGLE) << message;
			int result = send(master.fd_array[i], stream.str().c_str(), stream.str().size(), 0);
			if (result == SOCKET_ERROR || result <= 0)
			{
				RemoveUser(master.fd_array[i]);
			}
		}
	}
}

//Gets the name associated with a socket
char* Server::GetName(SOCKET s)
{
	return acceptedSockets[s];
}

//Checks to see if a socket is registered
bool Server::UserRegistered(SOCKET s)
{
	return acceptedSockets[s] != nullptr;
}

//Opens the file and writes the buffer into the file
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

//Initialize the file
void Server::InitializeFile()
{
	fileName = (char*)"../log_server.txt";
	FILE* pFile;
	errno_t err = fopen_s(&pFile, fileName, "w");
	if (pFile) fclose(pFile);
	int t = 0;
	//fileName = (const char*)("log_" + "t");
}

//Initialize the server with bind and listen
void Server::InitializeServer()
{
	while (true) //Loop until completed successfully
	{

		//Input
		maxClients = static_cast<uint8_t>(ReadInteger("Max number of clients: (0-99)", 0, 99));
		port = static_cast<uint16_t>(ReadInteger("Enter a port: 0-99999", 0, 99999));


		//Create socket
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (listenSocket == SOCKET_ERROR) // Error check
		{
			User::Clear();
			std::cout << "INIT SETUP ERROR: SERVER" << std::endl;
			return;
		}


		//Destination setup
		sockaddr_in sadd = {};
		sadd.sin_family = AF_INET;
		sadd.sin_port = htons(port);
		sadd.sin_addr.s_addr = INADDR_ANY;

		int result = bind(listenSocket, (SOCKADDR*)&sadd, sizeof(sadd));
		if (result == SOCKET_ERROR) // Error check
		{
			User::Clear();
			std::cout << "BIND ERROR: SERVER" << std::endl;
			return;
		}

		result = listen(listenSocket, 1); 
		if (result == SOCKET_ERROR) // Error check
		{
			User::Clear();
			std::cout << "LISTEN SETUP ERROR: SERVER" << std::endl;
		}
		break; //Completed successfully, break the infinite loop
	}

	//Final initialization
	FD_ZERO(&master);
	FD_ZERO(&read);

	FD_SET(listenSocket, &master);
}

//Prints to the console, and writes to the file for logging
void Server::PrintAndWrite(const char* buff)
{
	std::cout << buff;
	AddToFile(buff);
	oss = std::ostringstream();
}