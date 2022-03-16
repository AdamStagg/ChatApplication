// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the CLIENT_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// CLIENT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CLIENT_EXPORTS
#define CLIENT_API __declspec(dllexport)
#else
#define CLIENT_API __declspec(dllimport)
#endif

#include "../User/User.h"
#include <winsock.h>

class Client : public User
{
public:
	void Run();
private:
	SOCKET sock;
	std::string username;
	void Stop();
	void ConnectToServer();
	void sendMessage(SOCKET sock, char* buff, const int32_t length);
	void receiveEcho(SOCKET sock, char*& buff);
	void receiveMessage(bool* stopflag);
};

extern "C" CLIENT_API User* GenerateUser();
