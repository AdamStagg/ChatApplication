#pragma once
#define _GNU_SOURCE

#include <iostream>
#include "../Chat Application/Utils.h"
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include "../Chat Application/Utils.h"

class User
{
public:
    virtual void Run() = 0;
    enum class ErrorTypes
    {
        SUCCESS, 
        PARAMETER_ERROR,
        DISCONNECT,
        SHUTDOWN,
        COUNT
    };
    enum class MessageTypes
    {
        MESSAGE = 100,
        COMMAND,
        COUNT
    };
    enum class CommandTypes
    {
        REGISTER = 10,
        GETLOG,
        GETLIST,
        EXIT,
        COUNT
    };
private:
    virtual void Stop() = 0;
protected:
    void Clear()
    { 
        std::cout << "\x1B[2J\x1B[H";
    }
    ErrorTypes ErrorCheck(int input)
    {
        if ((input == SOCKET_ERROR) || input == 0)
        {
            if (WSAGetLastError() == WSAESHUTDOWN) return ErrorTypes::SHUTDOWN;
            else return ErrorTypes::DISCONNECT;
        }
    }

};
