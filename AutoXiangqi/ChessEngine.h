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
        ChessEngine(std::string engineName, std::string installPath, IPC& ipc);
        virtual void InitEngine() = 0;
        virtual AXQResult Run() = 0;

    public:
        IPC& m_IPC;
        PROCESS_INFORMATION pi;
        STARTUPINFO si;
    protected:
        std::string m_EngineName;
        std::string m_InstallPath;
    };

    class Pikafish : public ChessEngine
    {
    public:
        Pikafish(std::string engineName, std::string installPath, IPC& ipc);
        void InitEngine();
        AXQResult Run();
    };
}

#endif