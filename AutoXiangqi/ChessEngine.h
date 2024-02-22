#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include "AXQDefine.h"
#include "IPC.h"
#include <Windows.h>
#include <string>

namespace axq
{
    class ChessEngine
    {
    public:
        ChessEngine(std::string engineName, std::string installPath);
        virtual void InitEngine(HANDLE ChildReadNode, HANDLE ChildWriteNode) = 0;
        virtual AXQResult Run() = 0;

    public:
        PROCESS_INFORMATION pi;
        STARTUPINFO si;
    protected:
        std::string m_EngineName;
        std::string m_InstallPath;
    };

    class Pikafish : public ChessEngine
    {
    public:
        Pikafish(std::string engineName, std::string installPath);
        void InitEngine(HANDLE ChildReadNode, HANDLE ChildWriteNode);
        AXQResult Run();
    };
}

#endif