#ifndef IPC_H
#define IPC_H

#include "AXQDefine.h"
#include <Windows.h>
#include <string>

namespace axq
{
    class IPC
    {
    public:
        AXQResult InitIPC();

        AXQResult Read(char buff[], DWORD size, DWORD& readBytes);

        AXQResult Write(std::string cmd);

    public:
        HANDLE ParentWriteNode = nullptr;
        HANDLE ParentReadNode = nullptr;
        HANDLE ChildReadNode = nullptr;
        HANDLE ChildWriteNode = nullptr;
    private:
        SECURITY_ATTRIBUTES sa;
    };
}
#endif
