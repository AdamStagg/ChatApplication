// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the SERVER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// SERVER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SERVER_EXPORTS
#define SERVER_API __declspec(dllexport)
#else
#define SERVER_API __declspec(dllimport)
#endif

#include <vector>
#include "../User/User.h"

class Server : public User
{
public:
	void Run();
private:
	SOCKET listenSocket;
	std::vector<SOCKET> acceptedSockets;
	void Stop();
	FD_SET master;
	FD_SET read;
	FD_SET write;
	void readMessage(SOCKET sock, char*& buff);
};

extern "C" SERVER_API User * GenerateUser();