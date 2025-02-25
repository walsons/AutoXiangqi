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
        virtual void InitEngine() = 0;
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
        void InitEngine();
        AXQResult Run();
        inline static std::string m_Exe = "pikafish_x86-64-vnni256.exe";
    };

    class Growfish : public ChessEngine
    {
    public:
        Growfish(std::string engineName, std::string installPath);
        void InitEngine();
        AXQResult Run();
        inline static std::string m_Exe = "growfish.exe";
    };
}

#endif