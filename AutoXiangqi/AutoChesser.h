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
    enum class EngineType
    {
        GROWFISH,
        PIKAFISH
    };

	enum class RunType
	{
		MOVE_PIECE_BY_MESSAGE = 1,
		MOVE_PIECE_BY_MOUSE,
		MOVE_PIECE_LIKE_HUMAN
	};

    class AutoChesser
    {
    public:
        AutoChesser(const std::string& settingFileName = "setting.txt", EngineType engineType = EngineType::GROWFISH);
        AXQResult Run();

    private:
        // Option function
        AXQResult GetGameWindowClassName();
        AXQResult LocateChessBoard(bool topLeft);
        AXQResult LocateGameTimer(bool topLeft);
        AXQResult LocateNextGameButton(bool topLeft);
        AXQResult RecordPieceAppearance();
        AXQResult RecordTimerAppearance();
        AXQResult RecordNextGameButtonAppearance();
        AXQResult SaveConfig();
        AXQResult SetGameWindowPosition();
        AXQResult AutoPlayChess();
        AXQResult PlayChess();

    private:
        template <typename Engine>
        AXQResult ConfigureEngine(ChessEngine& engine);
        AXQResult UpdateConfigMember();
        AXQResult InvokeConfig();
        void CheckMyTurn(int interval, RunType runType);
        int FenSymbol(const std::string& fen);
        AXQResult MovePiece(RunType runType) 
        {
            std::string placeholder;
            return MovePiece(runType, placeholder);
        }
        AXQResult MovePiece(RunType runType, std::string& decidedMove);
        void MovePieceByMessage(POINT from, POINT to);
        void MovePieceByMouse(POINT from, POINT to);
        void MovePieceLikeHuman(POINT from, POINT to);
        void ImitateHumanMove(POINT destination, int sleepTime = 10, long stepSize = 5);

    public:
        FenGenerator m_FenGen;
        bool m_ActiveBash = false;
        EngineType m_EngineType;

        std::string fen_cache;
        bool time_to_move;

    private:
        std::string m_SettingFileName;
        std::fstream m_RW;
        std::string m_ConfigFileName = "Config.txt";
        std::unordered_map<std::string, std::string> m_Config;
        RunType m_RunType = RunType::MOVE_PIECE_BY_MESSAGE;
        ChessEngine* m_Engine = nullptr;
        std::atomic<bool> m_KeepCheck = false;
        std::future<void> m_AutoPlayChessThread;

        std::string m_GameWindowClassName;
        POINT m_ChessBoardTopLeft = { 0, 0 };
        POINT m_ChessBoardBottomRight = { 0, 0 };
        POINT m_GameTimerTopLeft = { 0, 0 };
        POINT m_GameTimerBottomRight = { 0, 0 };
        POINT m_OneMoreGameTopLeft = { 0, 0 };
        POINT m_OneMoreGameBottomRight = { 0, 0 };
        std::string m_ChessBoardPhotoFileName = "ChessBoardPhoto.png";
        std::string m_GameTimerPhotoFileName = "GameTimerPhoto.png";
        std::string m_NextGamePhotoFileName = "NextGamePhoto.png";
        HWND m_GameWindow = nullptr;


        char engineOutput[BuffSize + 1];
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

		//ipc.Write("setoption name Debug Log File value engine_log.txt");
		ipc.Write("setoption name Threads value 6");
		ipc.Write("setoption name Hash value 256");
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