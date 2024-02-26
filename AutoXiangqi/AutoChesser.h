#ifndef AUTO_CHESSER_HPP
#define AUTO_CHESSER_HPP

#include "AXQDefine.h"
#include "ChessEngine.h"
#include "IPC.h"
#include "FenGenerator.h"
#include <iostream>
#include <fstream>

namespace axq
{
	enum class RunType
	{
		MOVE_PIECE_BY_MESSAGE = 1,
		MOVE_PIECE_BY_MOUSE,
		MOVE_PIECE_LIKE_HUMAN
	};

	class AutoChesser
	{
	public:
		AutoChesser(bool readSetting = false, std::string settingName = "setting.txt");

		template <typename Engine>
		AXQResult ConfigureEngine(Engine& engine, IPC& ipc);

		AXQResult LocateChessBoard();

		AXQResult LocateWindow();

		AXQResult Run(IPC& ipc, FenGenerator& fenGen, RunType runType);

	private:
		void MovePieceByMessage(POINT from, POINT to);
		void MovePieceByMouse(POINT from, POINT to);
		void MovePieceLikeHuman(POINT from, POINT to);
		void ImitateHumanMove(POINT destination, int sleepTime = 10, long stepSize = 5);

	public:
		POINT topLeft = { 0, 0 };
		POINT bottomRight = { 0, 0 };
		HWND gameWindow = nullptr;
		HWND bashWindow = nullptr;
		bool activeBash = false;
		RECT windowRect = { 0, 0, 0, 0 };

	private:
		char engineOutput[BuffSize + 1];
		std::fstream settingMgr;
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
		ipc.Write("setoption name EvalFile value pikafish.nnue");

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