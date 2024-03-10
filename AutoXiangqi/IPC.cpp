#include "IPC.h"

#include <iostream>

namespace axq
{
    IPC& IPC::GetIPC()
    {
        static IPC ipc;
        return ipc;
    }

    bool IPC::Peek(char buff[], DWORD size, DWORD& readBytes)
    {
        DWORD avail = 0, left = 0;
        auto ret = PeekNamedPipe(ParentReadNode, buff, size, &readBytes, &avail, &left);
        if (avail > 0)
            return true;
        return false;
    }

    AXQResult IPC::Read(char buff[], DWORD size, DWORD& readBytes)
    {
        auto ret = ReadFile(ParentReadNode, buff, size, &readBytes, NULL);
        return ret ? AXQResult::ok : AXQResult::fail;
    }

    AXQResult IPC::Write(std::string cmd)
    {
        cmd.push_back('\n');
        DWORD writeBytes;
        auto ret = WriteFile(ParentWriteNode, cmd.c_str(), cmd.size(), &writeBytes, NULL);
        return ret ? AXQResult::ok : AXQResult::fail;
    }

    IPC::~IPC()
    {
        if (ParentWriteNode)
        {
            CloseHandle(ParentWriteNode);
            ParentWriteNode = nullptr;
        }
        if (ParentReadNode)
        {
            CloseHandle(ParentReadNode);
            ParentReadNode = nullptr;
        }
        if (ChildReadNode)
        {
            CloseHandle(ChildReadNode);
            ChildReadNode = nullptr;
        }
        if (ChildWriteNode)
        {
            CloseHandle(ChildWriteNode);
            ChildWriteNode = nullptr;
        }
    }

    IPC::IPC()
    {
        auto ret = InitIPC();
        if (ret != axq::AXQResult::ok)
        {
            std::cout << "Error: IPC::InitIPC()" << std::endl;
        }
    }

    AXQResult IPC::InitIPC()
    {
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        // Set the bInheritHandle flag so pipe handles are inherited.
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&ParentReadNode, &ChildWriteNode, &sa, 0))
            return AXQResult::fail;
        // Ensure the read handle to the pipe for STDOUT is not inherited.
        if (!SetHandleInformation(ParentReadNode, HANDLE_FLAG_INHERIT, 0))
            return AXQResult::fail;
        if (!CreatePipe(&ChildReadNode, &ParentWriteNode, &sa, 0))
            return AXQResult::fail;
        // Ensure the write handle to the pipe for STDIN is not inherited.
        if (!SetHandleInformation(ParentWriteNode, HANDLE_FLAG_INHERIT, 0))
            return AXQResult::fail;

        return AXQResult::ok;
    }
}