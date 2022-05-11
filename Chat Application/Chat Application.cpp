//#include "../Client/Client.h"
//#include "../Server/Server.h"
#include "Utils.h"
#include "../User/User.h"
#include <Windows.h>


enum class ChatModes
{
	Server,
	Client,
	COUNT
};

int main()
{
	//Get user input for the mode
    ChatModes chatMode = static_cast<ChatModes>(ReadInteger("Pick an option:\n1. Server\n2. Client", 1, 2) - 1);

	//Variable initialization
	User* user = nullptr;
	HINSTANCE hGetProcIDDLL;

	//Load the DLL
	switch (chatMode)
	{
	case ChatModes::Server:
		hGetProcIDDLL = LoadLibraryW(L"Server.dll");
		break;
	default:
	case ChatModes::COUNT:
	case ChatModes::Client:
		hGetProcIDDLL = LoadLibraryW(L"Client.dll");
		break;
	}

	//Error check the DLL
	if (!hGetProcIDDLL)
	{
		std::cout << "Could not load the library." << std::endl;
		return EXIT_FAILURE;
	}
	
	//Get the GenerateUser function from the DLL
	typedef User* (*uptr)();
	uptr GenerateUserFunction = reinterpret_cast<uptr>(GetProcAddress(hGetProcIDDLL, "GenerateUser"));
	
	//Error check the function
	if (!GenerateUserFunction)
	{
		std::cout << "Function not found in the library." << std::endl;
		return EXIT_FAILURE;
	}

	//Call the function
	user = GenerateUserFunction();
	
	//Socket setup
	WSADATA data;
	int ret = WSAStartup(WINSOCK_VERSION, &data);
	if (ret == SOCKET_ERROR)
		return -1;

	//Run the program
	user->Run();

	//Cleanup
	delete user;

	return 0;
}

