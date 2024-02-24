/*********************/
#include "IPC.h"
#include "ChessEngine.h"
#include "AutoChesser.h"
#include "ChessEngine.h"
#include "FenGenerator.h"

#include <Windows.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include <algorithm>
/*********************/

int ErrorExit(const std::string& errorInfo)
{
	std::cerr << errorInfo << std::endl;
	return -1;
}

template <typename... Handles>
void CloseHandles(Handles... handles)
{
	(void(), ..., CloseHandle(handles));
}

int main()
{
	axq::IPC ipc;
	auto ret = ipc.InitIPC();
	if (ret != axq::AXQResult::ok)
		return ErrorExit("ipc.InitIPC()");

	axq::Pikafish engine("pikafish", "pikafish_x86-64-vnni256.exe");
	engine.InitEngine(ipc.ChildReadNode, ipc.ChildWriteNode);
	ret = engine.Run();
	if (ret != axq::AXQResult::ok)
		return ErrorExit("engine.run()");

	axq::AutoChesser autoChesser;
	ret = autoChesser.ConfigureEngine<axq::Pikafish>(engine, ipc);
	if (ret != axq::AXQResult::ok)
		return ErrorExit("autoChesser.ConfigureEngine<axq::Pikafish>(engine, ipc)");
	ret = autoChesser.LocateChessBoard();
	if (ret != axq::AXQResult::ok)
		return ErrorExit("autoChesser.LocateChessBoard()");

	axq::FenGenerator fenGen(autoChesser.topLeft, autoChesser.bottomRight);
	cv::Mat boardScreenShot;
	fenGen.BoardScreenShot(boardScreenShot);
	fenGen.MakePieceFingerPrint(boardScreenShot);

	autoChesser.activeBash = true;
	autoChesser.LocateWindow();
	autoChesser.Run(ipc, fenGen, axq::RunType::MOVE_PIECE_BY_MESSAGE);

	// CloseHandles(pi.hProcess, pi.hThread, ParentWriteNode, ParentReadNode, ChildReadNode, ChildWriteNode);
	//system("pause");
	return 0;
}