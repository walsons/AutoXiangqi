#include "AutoChesser.h"
#include "Tools.h"

#include <future>

namespace axq
{
	AutoChesser::AutoChesser(const std::string& settingFileName)
		: m_SettingFileName(settingFileName)
	{
	}

	AXQResult AutoChesser::Run(RunType runType)
	{
		// Config
		std::cout << "1. Get game window class name:" << std::endl;
		std::cout << "\tPut the mouse cursor in the game window and and input \"cn\"(class name)" << std::endl;
		std::cout << "2. Locate chess board:" << std::endl;
		std::cout << "\tPut the mouse cursor in the board top left and input \"btl\"(board top left)" << std::endl;
		std::cout << "\tPut the mouse cursor in the board bottom right and input \"bbr\"(board bottom right)" << std::endl;
		std::cout << "3. Locate timer:" << std::endl;
		std::cout << "\tPut the mouse cursor in the timer top left and input \"ttl\"(timer top left)" << std::endl;
		std::cout << "\tPut the mouse cursor in the timer bottom right and input \"tbr\"(timer bottom right)" << std::endl;
		std::cout << "4. Record piece appearance:" << std::endl;
		std::cout << "\tInput \"rp\"(record piece)" << std::endl;
		std::cout << "5. Record timer appearance:" << std::endl;
		std::cout << "\tInput \"rt\"(record timer)" << std::endl;
		std::cout << "6. Save Config:" << std::endl;
		std::cout << "\tInput \"save\"" << std::endl;
		// Other setting
		std::cout << "7. Set the game window in the top right of the desktop" << std::endl;
		std::cout << "\tPut the mouse in the game window title bar and input \"pos\"(position)" << std::endl;
		// Exit
		std::cout << "Exit application:" << std::endl;
		std::cout << "\tInput exit" << std::endl;
		std::cout << std::endl;
		// Main command
		std::cout << "There are three type to run, default is 1" << std::endl;
		std::cout << "\t1. Input r1 to move piece by message" << std::endl;
		std::cout << "\t2. Input r2 to move piece by mouse" << std::endl;
		std::cout << "\t3. Input r3 to move piece like human" << std::endl;
		std::cout << "Input \"a\"(auto) to move chess automatically without command, input \"q\" to quit" << std::endl;

		// Read settings
		m_RW.open(m_ConfigFileName, std::ios::in);
		if (m_RW.good())
		{
			std::string line;
			while (std::getline(m_RW, line))
			{
				size_t pos = line.find("=");
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 1);
				m_Config.insert({ key, value });
			}
		}
		else
		{
			std::cout << "Setting file doesn't exist, create a new setting file" << std::endl;
			m_RW.open(m_ConfigFileName, std::ios::out);
		}
		m_RW.close();
		UpdateConfigMember();

		std::string cmd;
		while (std::cin >> cmd)
		{
			if (cmd == "cn")
			{
				GetGameWindowClassName();
				UpdateConfigMember();
			}
			else if (cmd == "btl")
			{
				LocateChessBoard(true);
				UpdateConfigMember();
			}
			else if (cmd == "bbr")
			{
				LocateChessBoard(false);
				UpdateConfigMember();
			}
			else if (cmd == "ttl")
			{
				LocateGameTimer(true);
				UpdateConfigMember();
			}
			else if (cmd == "tbr")
			{
				LocateGameTimer(false);
				UpdateConfigMember();
			}
			else if (cmd == "rp")
			{
				RecordPieceAppearance();
			}
			else if (cmd == "rt")
			{
				RecordTimerAppearance();
			}
			else if (cmd == "save")
			{
				SaveConfig();
			}
			else if (cmd == "pos")
			{
				SetGameWindowPosition();
			}
			else if (cmd == "exit")
			{
				break;
			}
			else if (cmd == "r1")
			{
				m_RunType = RunType::MOVE_PIECE_BY_MESSAGE;
			}
			else if (cmd == "r2")
			{
				m_RunType = RunType::MOVE_PIECE_BY_MOUSE;
			}
			else if (cmd == "r3")
			{
				m_RunType = RunType::MOVE_PIECE_LIKE_HUMAN;
			}
			else if (cmd == "a")
			{
				AutoPlayChess();
			}
			else if (cmd == "q")
			{
				m_KeepCheck = false;
			}
		}

		auto& ipc = IPC::GetIPC();

		std::cout << "Setting is ready" << std::endl;
		std::cout << "Input ng (new game) to start a new game" << std::endl;
		std::cout << "Input n (next) or ' (close to Enter) to get best move and move chess piece automatically" << std::endl;
		std::cout << "Input a (auto) to move chess automatically without command, input aq to quit" << std::endl;
		std::cout << "Input blank to make new blank piece finger print" << std::endl;
		std::future<void> fu;
		while (std::cin >> cmd)
		{
			if (cmd == "ng")
			{
				ipc.Write("ucinewgame");
			}
			else if (cmd == "a")
			{
				m_KeepCheck = true;
				fu = std::async(&AutoChesser::CheckMyTurn, this, 1000, runType);
			}
			else if (cmd == "q")
			{
				m_KeepCheck = false;
			}
			else if (cmd == "n" || cmd == "'")  // next
			{
				MovePiece(runType);
			}
			else if (cmd == "blank")
			{
				m_FenGen.MakeNewBlankPieceFingerPrint();
			}
		}
		return AXQResult::ok;
	}

	AXQResult AutoChesser::GetGameWindowClassName()
	{
		POINT pos;
		GetCursorPos(&pos);
		HWND hwnd = WindowFromPoint(pos);
		if (hwnd)
		{
			int maxCount = 1024;
			wchar_t* str = new wchar_t[maxCount];
			GetClassName(hwnd, str, maxCount);
			std::wcout << str << std::endl;
			m_Config["GameWindowClassName"] = wstring2string(str);
			delete[] str;
			return AXQResult::ok;
		}
		return AXQResult::fail;
	}

	AXQResult AutoChesser::LocateChessBoard(bool topLeft)
	{
		POINT pos;
		if (topLeft)
		{
			GetCursorPos(&pos);
			m_Config["ChessBoardTopLeft"] = std::to_string(pos.x) + "," + std::to_string(pos.y);
		}
		else
		{
			GetCursorPos(&pos);
			m_Config["ChessBoardBottomRight"] = std::to_string(pos.x) + "," + std::to_string(pos.y);
		}
		return AXQResult::ok;
	}

	AXQResult AutoChesser::LocateGameTimer(bool topLeft)
	{
		POINT pos;
		if (topLeft)
		{
			GetCursorPos(&pos);
			m_Config["GameTimerTopLeft"] = std::to_string(pos.x) + "," + std::to_string(pos.y);
		}
		else
		{
			GetCursorPos(&pos);
			m_Config["GameTimerBottomRight"] = std::to_string(pos.x) + "," + std::to_string(pos.y);
		}
		return AXQResult::ok;
	}

	AXQResult AutoChesser::RecordPieceAppearance()
	{
		cv::Mat chessBoardPhoto;
		m_FenGen.SnippingChessBoard(chessBoardPhoto, nullptr);
		cv::imwrite(m_ChessBoardPhotoFileName, chessBoardPhoto);
		return AXQResult::ok;
	}

	AXQResult AutoChesser::RecordTimerAppearance()
	{
		cv::Mat gameTimerPhoto;
		m_FenGen.GameTimerShot(gameTimerPhoto);
		cv::imwrite(m_GameTimerPhotoFileName, gameTimerPhoto);
		return AXQResult::ok;
	}

    AXQResult AutoChesser::SetGameWindowPosition()
    {
        POINT curPoint;
        GetCursorPos(&curPoint);
        HWND window = WindowFromPoint(curPoint);
        AXQResult ret = AXQResult::fail;
        if (window != nullptr)
        {
            if (SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE))
                ret = AXQResult::ok;
        }
        return ret;
    }

	AXQResult AutoChesser::SaveConfig()
	{
		// Write settings
		m_RW.open(m_ConfigFileName, std::ios::out);
		if (m_RW.is_open())
		{
			for (auto& item : m_Config)
			{
				m_RW << item.first << "=" << item.second << std::endl;
			}
		}
		else
		{
			std::cout << "Save config failed" << std::endl;
		}
		m_RW.close();
		return AXQResult::ok;
	}

    AXQResult AutoChesser::AutoPlayChess()
    {
		if (m_Engine == nullptr)
		{
			m_Engine = new Pikafish("pikafish", "pikafish_x86-64-vnni256.exe");
			m_Engine->InitEngine();
			auto ret = m_Engine->Run();
			if (ret != AXQResult::ok)
			{
				std::cout << "Start engine failed" << std::endl;
				return ret;
			}
			ret = ConfigureEngine<Pikafish>(*m_Engine);
			if (ret != AXQResult::ok)
			{
				std::cout << "Configure engine failed" << std::endl;
				return ret;
			}
		}

		m_FenGen.m_InputFen = Fen();
        m_KeepCheck = true;
        m_AutoPlayChessThread = std::async(&AutoChesser::CheckMyTurn, this, 1000, RunType::MOVE_PIECE_BY_MESSAGE);

        return AXQResult::ok;
    }

	AXQResult AutoChesser::UpdateConfigMember()
	{
		auto parsePoint = [](const std::string& point) {
			auto pos = point.find(",");
			if (pos != std::string::npos)
			{
				int x = std::stoi(point.substr(0, pos));
				int y = std::stoi(point.substr(pos + 1));
				return POINT{ x, y };
			}
			std::cout << "Error: Parse point failed" << std::endl;
			return POINT{ 0, 0 };
		};

		if (m_Config.find("GameWindowClassName") != m_Config.end())
			m_GameWindowClassName = m_Config["GameWindowClassName"];
		if (m_Config.find("ChessBoardTopLeft") != m_Config.end())
			m_ChessBoardTopLeft = parsePoint(m_Config["ChessBoardTopLeft"]);
		if (m_Config.find("ChessBoardBottomRight") != m_Config.end())
			m_ChessBoardBottomRight = parsePoint(m_Config["ChessBoardBottomRight"]);
		if (m_Config.find("GameTimerTopLeft") != m_Config.end())
			m_GameTimerTopLeft = parsePoint(m_Config["GameTimerTopLeft"]);
		if (m_Config.find("GameTimerBottomRight") != m_Config.end())
			m_GameTimerBottomRight = parsePoint(m_Config["GameTimerBottomRight"]);

		InvokeConfig();

		return AXQResult::ok;
	}

	AXQResult AutoChesser::InvokeConfig()
	{
		m_FenGen.m_ScreenShotTopLeft = m_ChessBoardTopLeft;
		m_FenGen.m_ScreenShotBottomRight = m_ChessBoardBottomRight;

		m_FenGen.m_GameTimerTopLeft = m_GameTimerTopLeft;
		m_FenGen.m_GameTimerBottomRight = m_GameTimerBottomRight;

		cv::Mat chessBoardShot = cv::imread(m_ChessBoardPhotoFileName, cv::IMREAD_GRAYSCALE);
		if (chessBoardShot.data == nullptr)
			return AXQResult::fail;
		m_FenGen.MakePieceFingerPrint(chessBoardShot);

		LPCWSTR className = string2wstring(m_GameWindowClassName).c_str();
		auto win = FindWindow(className, nullptr);
		// Currently only this method work, will find another way to find the window
		m_GameWindow = WindowFromPoint({ (m_ChessBoardTopLeft.x + m_ChessBoardBottomRight.x) / 2, (m_ChessBoardTopLeft.y + m_ChessBoardBottomRight.y) / 2 });

		return AXQResult::ok;
	}
	
	void AutoChesser::CheckMyTurn(int interval, RunType runType)
	{
		while (m_KeepCheck)
		{
			m_MyTurn = m_FenGen.IsMyTurn();
			if (m_MyTurn)
			{
				Sleep(1000);
				MovePiece(runType);
			}
			else
				Sleep(interval);
		}
	}

	int AutoChesser::FenSymbol(const std::string& fen)
	{
		char selfColor = fen[fen.size() - std::string("b - - 0 1").size()];
		int symbol = 0;
		for (auto c : fen)
		{
			switch (c)
			{
			case 'r':
				symbol += (1 << 13);
				break;
			case 'n':
				symbol += (1 << 12);
				break;
			case 'b':
				symbol += (1 << 11);
				break;
			case 'a':
				symbol += (1 << 10);
				break;
			case 'k':
				symbol += (1 << 9);
				break;
			case 'c':
				symbol += (1 << 8);
				break;
			case 'p':
				symbol += (1 << 7);
				break;
			case 'R':
				symbol += (1 << 6);
				break;
			case 'N':
				symbol += (1 << 5);
				break;
			case 'B':
				symbol += (1 << 4);
				break;
			case 'A':
				symbol += (1 << 3);
				break;
			case 'K':
				symbol += (1 << 2);
				break;
			case 'C':
				symbol += (1 << 1);
				break;
			case 'P':
				symbol += (1 << 0);
				break;
			default:
				break;
			}
		}
		if (selfColor == 'b')
			symbol -= (1 << 4);
		return symbol;
	}

	void AutoChesser::MovePiece(RunType runType)
	{
		// Scan board to fen
		std::string fen = m_FenGen.GenerateFen();
		// Invalid fen
		if (fen.empty())
		{
			m_FenGen.m_InputFen = Fen();
			return;
		}
		int symbol1 = FenSymbol(fen);
		int symbol2 = FenSymbol(m_FenGen.m_InputFen.GetReal());
		std::cout << "symbol1: " << symbol1 << ",   " << "symbol2: " << symbol2 << std::endl;
		if (symbol1 != symbol2)
		{
			Sleep(1800);
			//std::cout << "symbol1: " << symbol1 << ",   " << "symbol2: " << symbol2 << std::endl;
			fen = m_FenGen.GenerateFen();
			if (fen.empty())
			{
				m_FenGen.m_InputFen = Fen();
				return;
			}
			m_FenGen.m_InputFen = Fen(fen);
		}
		else
		{
			auto enemyMove = Fen(fen) - m_FenGen.m_InputFen;
			m_FenGen.m_InputFen.push(enemyMove);
		}

		auto& ipc = IPC::GetIPC();
		ipc.Write("position fen " + m_FenGen.m_InputFen.Get());
		std::cout << (m_FenGen.m_InputFen.Get()) << std::endl;
		std::cout << (m_FenGen.m_InputFen.GetReal()) << std::endl;
		ipc.Write("go depth 15");


		std::string bestMove;
		std::string output;
		while (true)
		{
			// Check engine is still alive
			DWORD engineResult = 0;
			GetExitCodeProcess(m_Engine->pi.hProcess, &engineResult);
			if (engineResult != STILL_ACTIVE)
			{
				if (m_Engine != nullptr)
				{
					delete m_Engine;
					m_Engine = nullptr;
				}
				m_Engine = new Pikafish("pikafish", "pikafish_x86-64-vnni256.exe");
				m_Engine->InitEngine();
				auto ret = m_Engine->Run();
				if (ret != AXQResult::ok)
				{
					std::cout << "start engine failed" << std::endl;
					return;
				}
				ret = ConfigureEngine<Pikafish>(*m_Engine);
				if (ret != AXQResult::ok)
				{
					std::cout << "start engine failed" << std::endl;
					return;
				}
				std::cout << "Engine restart" << std::endl;
				m_FenGen.m_InputFen = Fen();
				return;
			}

			DWORD readBytes = 0;
			if (ipc.Peek(engineOutput, BuffSize, readBytes))
			{
				ipc.Read(engineOutput, BuffSize, readBytes);
			}
			engineOutput[readBytes] = '\0';
			output += engineOutput;
			auto pos = output.find("bestmove");
			if (pos != std::string::npos && (pos + std::string("bestmove xxxx").size() <= output.size()))
			{
				bestMove = output.substr(pos + std::string("bestmove ").size(), std::string("xxxx").size());
				if (bestMove[0] >= 'a' && bestMove[0] <= 'i' &&
					bestMove[1] >= '0' && bestMove[1] <= '9' &&
					bestMove[2] >= 'a' && bestMove[2] <= 'i' &&
					bestMove[3] >= '0' && bestMove[3] <= '9')
				{
					m_FenGen.m_InputFen.push(bestMove);
					break;
				}
				else
				{
					m_FenGen.m_InputFen = Fen();
					return;
				}
			}
		}
		if (fen[fen.size() - std::string("b - - 0 1").size()] == 'b')
		{
			for (auto& c : bestMove)
			{
				if (c >= '0' && c <= '9')
					c = ('9' - (c - '0'));
				else
					c = ('i' - (c - 'a'));
			}
			std::cout << "black bestMove: " << bestMove << std::endl;
		}
		else
			std::cout << "red bestMove: " << bestMove << std::endl;

		int col1 = bestMove[0] - 'a';
		int row1 = '9' - bestMove[1];
		int col2 = bestMove[2] - 'a';
		int row2 = '9' - bestMove[3];
		auto& bc = m_FenGen.boardCoordinate;
		auto dpi = m_FenGen.GetWindowDpi();
		POINT from{ bc[row1][col1].x / dpi + m_FenGen.m_ScreenShotTopLeft.x, bc[row1][col1].y / dpi + m_FenGen.m_ScreenShotTopLeft.y };
		POINT to{ bc[row1][col2].x / dpi + m_FenGen.m_ScreenShotTopLeft.x, bc[row2][col2].y / dpi + m_FenGen.m_ScreenShotTopLeft.y };

		switch (runType)
		{
		case RunType::MOVE_PIECE_BY_MESSAGE:
			MovePieceByMessage(from, to);
			break;
		case RunType::MOVE_PIECE_BY_MOUSE:
			MovePieceByMouse(from, to);
			break;
		case RunType::MOVE_PIECE_LIKE_HUMAN:
			MovePieceLikeHuman(from, to);
			break;
		default:
			MovePieceByMessage(from, to);
			break;
		}

		if (activeBash)
		{
			// give some time for mouse event to change active window
			Sleep(500);
			RECT rect;
			GetWindowRect(GetConsoleWindow(), &rect);
			POINT originPos;
			GetCursorPos(&originPos);
			SetCursorPos(rect.left + 20, rect.bottom - 20);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			SetCursorPos(originPos.x, originPos.y);
		}
		// give time to game to play animation
		Sleep(2500);
	}

	void AutoChesser::MovePieceByMessage(POINT from, POINT to)
	{
		auto gameWindow = WindowFromPoint({ (m_ChessBoardTopLeft.x + m_ChessBoardBottomRight.x) / 2, (m_ChessBoardTopLeft.y + m_ChessBoardBottomRight.y) / 2 });
		RECT rect;
		GetWindowRect(gameWindow, &rect);
		SendMessage(gameWindow, WM_LBUTTONDOWN, 0, from.x - rect.left + ((from.y - rect.top) << 16));
		SendMessage(gameWindow, WM_LBUTTONUP, 0, from.x - rect.left + ((from.y - rect.top) << 16));
		Sleep(500);
		SendMessage(gameWindow, WM_LBUTTONDOWN, 0, to.x - rect.left + ((to.y - rect.top) << 16));
		SendMessage(gameWindow, WM_LBUTTONUP, 0, to.x - rect.left + ((to.y - rect.top) << 16));
		// Ensure piece move to destination
		Sleep(100);
		SendMessage(gameWindow, WM_LBUTTONDOWN, 0, to.x - rect.left + ((to.y - rect.top) << 16));
		SendMessage(gameWindow, WM_LBUTTONUP, 0, to.x - rect.left + ((to.y - rect.top) << 16));
	}

	void AutoChesser::MovePieceByMouse(POINT from, POINT to)
	{
		SetCursorPos(from.x, from.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);
		SetCursorPos(to.x, to.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		// Ensure piece move to destination
		Sleep(100);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}

	void AutoChesser::MovePieceLikeHuman(POINT from, POINT to)
	{
		ImitateHumanMove(from);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);
		ImitateHumanMove(to);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		// Ensure piece move to destination
		Sleep(100);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}

	void AutoChesser::ImitateHumanMove(POINT destination, int sleepTime, long stepSize)
	{
		POINT begin, end;
		GetCursorPos(&begin);
		end = destination;

		long absXMove = std::abs(end.x - begin.x);
		long absYMove = std::abs(end.y - begin.y);
		bool flag = false;
		long m = 0, n = 0;
		if (absXMove >= absYMove)
		{
			flag = true;
			m = absXMove;
			n = absYMove;
		}
		else
		{
			flag = false;
			n = absXMove;
			m = absYMove;
		}

		double rate = static_cast<double>(n) / m;
		long step = 1;
		std::vector<long> mArr, nArr;
		while (m > 2 * step)
		{
			m -= 2 * step;
			mArr.push_back(step);
			long nStep = std::floor(step * rate);
			n -= 2 * nStep;
			nArr.push_back(nStep);
			step += stepSize;
		}

		long mInter = m;
		long nInter = n;
		std::vector<long> xArr(mArr.size(), 0), yArr(mArr.size(), 0);
		long xInter = 0, yInter = 0;
		if (flag)
		{
			if (end.x >= begin.x)
			{
				xArr = mArr;
				xInter = mInter;
			}
			else
			{
				std::transform(mArr.begin(), mArr.end(), xArr.begin(), [](int value) {
					return -value;
					});
				xInter = -mInter;
			}
			if (end.y >= begin.y)
			{
				yArr = nArr;
				yInter = nInter;
			}
			else
			{
				std::transform(nArr.begin(), nArr.end(), yArr.begin(), [](int value) {
					return -value;
					});
				yInter = -nInter;
			}
		}
		else
		{
			if (end.x >= begin.x)
			{
				xArr = nArr;
				xInter = nInter;
			}
			else
			{
				std::transform(nArr.begin(), nArr.end(), xArr.begin(), [](int value) {
					return -value;
					});
				xInter = -nInter;
			}
			if (end.y >= begin.y)
			{
				yArr = mArr;
				yInter = mInter;
			}
			else
			{
				std::transform(mArr.begin(), mArr.end(), yArr.begin(), [](int value) {
					return -value;
					});
				yInter = -mInter;
			}
		}

		// Start move
		POINT b = begin, e = end;
		auto mouseMove = [&](long x, long y) {
			b.x += x;
			b.y += y;
			SetCursorPos(b.x, b.y);
		};
		for (size_t i = 0; i < mArr.size(); ++i)
		{
			mouseMove(xArr[i], yArr[i]);
			Sleep(sleepTime);
		}
		mouseMove(xInter, yInter);
		Sleep(sleepTime);
		for (int i = static_cast<int>(xArr.size()) - 1; i >= 0; --i)
		{
			mouseMove(xArr[i], yArr[i]);
			Sleep(sleepTime);
		}
		// Ensure the cursor is set to destination
		SetCursorPos(end.x, end.y);
	}
}
