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
		AutoChesser(ChessEngine* chessEngine, const std::string& settingFileName = "setting.txt");

		template <typename Engine>
		AXQResult ConfigureEngine(ChessEngine& engine);

		AXQResult ConfigureSetting();

		AXQResult Run(RunType runType);

	private:
		AXQResult SetGameWindowPos();
		AXQResult AnalyzeChessBoard();
		AXQResult LocateGameTimer();
		AXQResult LocateWindow();
		int FenSymbol(const std::string& fen);

		void CheckMyTurn(int interval, RunType runType);
		void MovePiece(RunType runType);

		void MovePieceByMessage(POINT from, POINT to);
		void MovePieceByMouse(POINT from, POINT to);
		void MovePieceLikeHuman(POINT from, POINT to);
		void ImitateHumanMove(POINT destination, int sleepTime = 10, long stepSize = 5);

	public:
		FenGenerator m_FenGen;
		HWND gameWindow = nullptr;
		HWND bashWindow = nullptr;
		bool activeBash = false;
		RECT windowRect = { 0, 0, 0, 0 };

	private:
		char engineOutput[BuffSize + 1];
		std::fstream m_RW;
		bool m_ReadSetting = false;
		std::string m_SettingFileName;
		std::unordered_map<std::string, std::string> m_Settings;
		std::atomic<bool> m_MyTurn = false;
		std::atomic<bool> m_KeepCheck = false;
		std::string m_LastFen = "b - - 0 1";
		ChessEngine* m_Engine;
	};

	template <>
	inline AXQResult AutoChesser::ConfigureEngine<Pikafish>(ChessEngine& engine)
	{
		const int BuffSize = 4096;
		char engineOutput[BuffSize + 1];
		DWORD readBytes = 0;
		DWORD writeBytes = 0;

		auto& ipc = IPC::GetIPC();
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