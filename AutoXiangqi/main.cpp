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
	axq::ChessEngine* engine = new axq::Pikafish("pikafish", "pikafish_x86-64-vnni256.exe");
	engine->InitEngine();
	auto ret = engine->Run();
	if (ret != axq::AXQResult::ok)
		return ErrorExit("engine.run()");

	axq::AutoChesser autoChesser(engine);
	ret = autoChesser.ConfigureEngine<axq::Pikafish>(*engine);
	if (ret != axq::AXQResult::ok)
		return ErrorExit("autoChesser.ConfigureEngine<axq::Pikafish>(engine, ipc)");
	ret = autoChesser.ConfigureSetting();
	if (ret != axq::AXQResult::ok)
		return ErrorExit("autoChesser.ConfigureSetting()");
	// autoChesser.activeBash = true;
	autoChesser.Run(axq::RunType::MOVE_PIECE_BY_MESSAGE);

	auto ipc = axq::IPC::GetIPC();
	CloseHandles(engine->pi.hProcess, engine->pi.hThread);
	//system("pause");
	return 0;
}