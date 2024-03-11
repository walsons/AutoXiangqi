#ifndef AUTO_CHESSER_HPP
#define AUTO_CHESSER_HPP

#include "AXQDefine.h"
#include "ChessEngine.h"
#include "IPC.h"
#include "FenGenerator.h"

#include <iostream>
#include <fstream>
#include <future>

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
		AutoChesser(const std::string& settingFileName = "setting.txt");
		AXQResult Run(RunType runType);

	private:
		// Option function
		AXQResult GetGameWindowClassName();
		AXQResult LocateChessBoard(bool topLeft);
		AXQResult LocateGameTimer(bool topLeft);
		AXQResult RecordPieceAppearance();
		AXQResult RecordTimerAppearance();
		AXQResult SaveConfig();
        AXQResult SetGameWindowPosition();
        AXQResult AutoPlayChess();
		
	private:
		template <typename Engine>
		AXQResult ConfigureEngine(ChessEngine& engine);
		AXQResult UpdateConfigMember();
		AXQResult InvokeConfig();
		void CheckMyTurn(int interval, RunType runType);
		int FenSymbol(const std::string& fen);
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
		std::unordered_map<std::string, std::string> m_Config;
        std::string m_ConfigFileName = "Config.txt";
        std::string m_GameWindowClassName;
        HWND m_GameWindow = nullptr;
        POINT m_ChessBoardTopLeft;
        POINT m_ChessBoardBottomRight;
        POINT m_GameTimerTopLeft;
        POINT m_GameTimerBottomRight;
        std::string m_ChessBoardPhotoFileName = "ChessBoardPhoto.png";
        std::string m_GameTimerPhotoFileName = "GameTimerPhoto.png";
        std::future<void> m_AutoPlayChessThread;

		char engineOutput[BuffSize + 1];
		std::fstream m_RW;
		bool m_ReadSetting = false;
		std::string m_SettingFileName;
		std::unordered_map<std::string, std::string> m_Settings;
		std::atomic<bool> m_MyTurn = false;
		std::atomic<bool> m_KeepCheck = false;
		std::string m_LastFen = "b - - 0 1";
		ChessEngine* m_Engine = nullptr;

		RunType m_RunType = RunType::MOVE_PIECE_BY_MESSAGE;;
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

		ipc.Write("setoption name Debug Log File value engine_log.txt");
		ipc.Write("setoption name Threads value 6");
		ipc.Write("setoption name Hash value 512");
		ipc.Write("setoption name Ponder value false");
		ipc.Write("setoption name MultiPV value 1");
		ipc.Write("setoption name Move Overhead value 10");
		ipc.Write("setoption name EvalFile value pikafish.nnue");

		ipc.Write("uci");
		ipc.Read(engineOutput, BuffSize, readBytes);
		engineOutput[readBytes] = '\0';
		std::cout << "Engine Info: " << engineOutput << std::endl;

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