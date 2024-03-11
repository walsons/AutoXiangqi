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
	/*Fen f("3akab2/9/8b/p1N2c3/9/2P1n4/P3p1P1P/B4C2N/4A4/3AK1B2 b - - 0 1 moves i3i4 d9e8 i2h4");
	f.push("h7h8");
	Fen f2("4kab2/4a4/8b/p1N2c3/9/2P1n2NP/P3p1P2/B4C3/4A4/3AK1B2 b - - 0 1");
	Fen t("3akab2/9/8b/p1N2c3/9/2P1n4/P3p1P1P/B4C2N/4A4/3AK1B2 b - - 0 1");
	std::cout << t.push("i3i4").push("d9e8").GetReal() << std::endl;
	std::cout << t.GetReal() << std::endl;*/


	//autoChesser.activeBash = true;
    axq::AutoChesser autoChesser;
	autoChesser.Run(axq::RunType::MOVE_PIECE_BY_MESSAGE);

	//auto ipc = axq::IPC::GetIPC();
	//CloseHandles(engine->pi.hProcess, engine->pi.hThread);
	//system("pause");
	return 0;
}