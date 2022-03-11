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
    ChatModes chatMode = static_cast<ChatModes>(ReadInteger("Pick an option:\n1. Server\n2. Client", 1, 2) - 1);

	User* user = nullptr;
	HINSTANCE hGetProcIDDLL;


	switch (chatMode)
	{
	case ChatModes::Server:
		hGetProcIDDLL = LoadLibrary(L"Server.dll");
		break;
	default:
	case ChatModes::COUNT:
	case ChatModes::Client:
		hGetProcIDDLL = LoadLibrary(L"Client.dll");
		break;
	}

	if (!hGetProcIDDLL)
	{
		std::cout << "Could not load the library." << std::endl;
		return EXIT_FAILURE;
	}

	typedef User* (*uptr)();
	uptr GenerateUserFunction = reinterpret_cast<uptr>((GetProcAddress(hGetProcIDDLL, "GenerateUser")));
	
	if (!GenerateUserFunction)
	{
		std::cout << "Function not found in the library." << std::endl;
		return EXIT_FAILURE;
	}

	user = GenerateUserFunction();
	
	WSADATA data;
	int ret = WSAStartup(WINSOCK_VERSION, &data);
	if (ret == SOCKET_ERROR)
		return -1;

	user->Run();

	delete user;

	return 0;
}

