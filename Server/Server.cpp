// Server.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Server.h"

void Server::Run()
{
	std::string username = ReadString("Enter a username:");
	
}

User* GenerateUser()
{
	return new Server();
}