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

	std::cout << "Read previous setting? input y or n" << std::endl;
	bool readSetting = false;
	std::string cmd;
	std::cin >> cmd;
	if (cmd == "y" || cmd == "Y")
		readSetting = true;
	axq::AutoChesser autoChesser(readSetting);
	ret = autoChesser.ConfigureEngine<axq::Pikafish>(engine, ipc);
	if (ret != axq::AXQResult::ok)
		return ErrorExit("autoChesser.ConfigureEngine<axq::Pikafish>(engine, ipc)");
	if (!readSetting)
	{
		ret = autoChesser.LocateChessBoard();
		if (ret != axq::AXQResult::ok)
			return ErrorExit("autoChesser.LocateChessBoard()");
		ret = autoChesser.LocateWindow();
		if (ret != axq::AXQResult::ok)
			return ErrorExit("autoChesser.LocateWindow()");
	}
	autoChesser.activeBash = true;

	axq::FenGenerator fenGen(autoChesser.topLeft, autoChesser.bottomRight);
	cv::Mat boardScreenShot;
	if (readSetting)
		boardScreenShot = cv::imread("setting.png", cv::IMREAD_GRAYSCALE);
	else
	{
		fenGen.BoardScreenShot(boardScreenShot);
		cv::imwrite("setting.png", boardScreenShot);
	}
	fenGen.MakePieceFingerPrint(boardScreenShot);

	autoChesser.Run(ipc, fenGen, axq::RunType::MOVE_PIECE_BY_MESSAGE);

	CloseHandles(engine.pi.hProcess, engine.pi.hThread, ipc.ParentWriteNode, ipc.ParentReadNode, ipc.ChildReadNode, ipc.ChildWriteNode);
	//system("pause");
	return 0;
}