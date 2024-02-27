#include "ChessEngine.h"
#include "Tools.h"
#include <iostream>
namespace axq
{
    ChessEngine::ChessEngine(std::string engineName, std::string installPath, IPC& ipc)
        : m_EngineName(engineName), m_InstallPath(installPath), m_IPC(ipc)
    {
    }

    Pikafish::Pikafish(std::string engineName, std::string installPath, IPC& ipc)
        : ChessEngine(engineName, installPath, ipc)
    {
    }

    void Pikafish::InitEngine()
    {
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.hStdInput = m_IPC.ChildReadNode;
        si.hStdError = m_IPC.ChildWriteNode;
        si.hStdOutput = m_IPC.ChildWriteNode;
        si.dwFlags |= STARTF_USESTDHANDLES;
    }

    AXQResult Pikafish::Run()
    {
        std::wstring wInstallPath = string2wstring(m_InstallPath);
        LPWSTR cmd = (LPWSTR)wInstallPath.c_str();
        auto ret = CreateProcess(NULL,
            cmd,
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &si,
            &pi);
        if (!ret)
            return AXQResult::fail;
        return AXQResult::ok;
    }
}
