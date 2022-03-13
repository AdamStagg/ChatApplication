#pragma once
#include <iostream>
#include "../Chat Application/Utils.h"
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

class User
{
public:
    virtual void Run() = 0;
private:
    virtual void Stop() = 0;
protected:
    void Clear()
    { 
        std::cout << "\x1B[2J\x1B[H";
    }
};
