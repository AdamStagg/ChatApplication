// Client.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Client.h"

void Client::Run()
{
	printf("THE DLL IS WORKING NOW");
}

User* GenerateUser()
{
	return new Client();
}

