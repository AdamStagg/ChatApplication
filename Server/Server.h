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

#include <map>
#include "../User/User.h"
#include <sstream>

class Server : public User
{
public:
	void Run();
private:
	char* fileName;
	int8_t maxClients;
	SOCKET listenSocket;
	int16_t port;
	std::ostringstream oss;
	std::map<SOCKET, char*> acceptedSockets;
	void Stop();
	FD_SET master;
	FD_SET read;
	void readMessage(SOCKET sock);
	void AddUser();
	void RemoveUser(SOCKET s);
	void echoMessage(char* buff);
	char* GetName(SOCKET s);
	bool UserRegistered(SOCKET s);
	void AddToFile(const char* buf);
	void InitializeFile();
	void InitializeServer();
	void PrintAndWrite(const char* buffToWrite);
};

extern "C" SERVER_API User * GenerateUser();