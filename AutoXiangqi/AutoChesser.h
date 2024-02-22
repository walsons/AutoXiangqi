#ifndef AUTO_CHESSER_HPP
#define AUTO_CHESSER_HPP

#include "AXQDefine.h"
#include "ChessEngine.h"
#include "IPC.h"
#include <iostream>

namespace axq
{
	class AutoChesser
	{
	public:
		template <typename Engine>
		AXQResult ConfigureEngine(Engine& engine, IPC& ipc);

		AXQResult LocateChessBoard();

		// Method 1, send message to window to move chess piece
		AXQResult Run1(bool activeBash);
		// Method 2, simulate mouse click to move chess piece
		AXQResult Run2(bool activeBash);

	public:
		POINT topLeft = { 0, 0 };
		POINT bottomRight = { 0, 0 };
		HWND gameWindow = nullptr;
	};

	template <>
	inline AXQResult AutoChesser::ConfigureEngine<Pikafish>(Pikafish& engine, IPC& ipc)
	{
		const int BuffSize = 4096;
		char engineOutput[BuffSize + 1];
		DWORD readBytes = 0;
		DWORD writeBytes = 0;

		ipc.Read(engineOutput, BuffSize, readBytes);
		engineOutput[readBytes] = '\0';
		std::cout << "Engine Info: " << engineOutput << std::endl;

		ipc.Write("uci");

		ipc.Read(engineOutput, BuffSize, readBytes);
		engineOutput[readBytes] = '\0';
		std::cout << "Engine Info: " << engineOutput << std::endl;

		ipc.Write("setoption name Threads value 4");
		ipc.Write("setoption name EvalFile value C:/Users/shayne/source/repos/AutoXiangqi/x64/Debug/pikafish.nnue");

		// Game start
		ipc.Write("ucinewgame");
		ipc.Write("isready");
		ipc.Read(engineOutput, BuffSize, readBytes);
		engineOutput[readBytes] = '\0';
		std::string isReady = engineOutput;
		if (isReady.find("readyok") == std::string::npos)
			return AXQResult::fail;
		return AXQResult::ok;
	}
}
#endif