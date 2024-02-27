#include "AutoChesser.h"

namespace axq
{
	AutoChesser::AutoChesser(IPC& ipc, const std::string& settingFileName)
		: m_IPC(ipc), m_SettingFileName(settingFileName)
	{
	}

	AXQResult AutoChesser::ConfigureSetting()
	{
		std::cout << "Read previous setting? input y or n" << std::endl;
		std::string cmd;
		std::cin >> cmd;
		if (cmd == "y")
			m_ReadSetting = true;
		if (m_ReadSetting)
			m_RW.open(m_SettingFileName, std::ios::in);
		else
			m_RW.open(m_SettingFileName, std::ios::out);
		if (!m_RW.is_open())
			std::cout << "cannot open setting file" << std::endl;
		if (m_ReadSetting)
		{
			std::string line;
			while (std::getline(m_RW, line))
			{
				size_t pos = line.find("=");
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 1);
				m_Settings.insert({ key, value });
			}
		}
		// step 1: set window position to (0, 0)
		SetGameWindowPos();
		// step 2 
		LocateChessBoard();
		m_FenGen.photoTopLeft = m_ScreenShotTopLeft;
		m_FenGen.photoBottomRight = m_ScreenShotBottomRight;
		cv::Mat boardScreenShot;
		if (m_ReadSetting)
			boardScreenShot = cv::imread("setting.png", cv::IMREAD_GRAYSCALE);
		else
		{
			m_FenGen.BoardScreenShot(boardScreenShot);
			cv::imwrite("setting.png", boardScreenShot);
		}
		m_FenGen.MakePieceFingerPrint(boardScreenShot);
		// step 3
		LocateWindow();
		
		return AXQResult::ok;
	}

	AXQResult AutoChesser::SetGameWindowPos()
	{
		std::string cmd;
		std::cout << "Please put the mouse in the game window title bar, input \"pos\" for sure" << std::endl;
		while (std::cin >> cmd)
		{
			if (cmd == "pos")
				break;
		}
		POINT curPoint;
		if (!GetCursorPos(&curPoint))
			return AXQResult::fail;
		HWND window = WindowFromPoint(curPoint);
		if (!SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE))
			return AXQResult::fail;
		return AXQResult::ok;
	}

	AXQResult AutoChesser::LocateChessBoard()
	{
		if (m_ReadSetting)
		{
			std::string value = m_Settings["m_ScreenShotTopLeft"];
			auto pos = value.find(",");
			if (pos != std::string::npos)
			{
				int x = std::stoi(value.substr(0, pos));
				int y = std::stoi(value.substr(pos + 1));
				m_ScreenShotTopLeft = { x, y };
			}
			value = m_Settings["m_ScreenShotBottomRight"];
			pos = value.find(",");
			if (pos != std::string::npos)
			{
				int x = std::stoi(value.substr(0, pos));
				int y = std::stoi(value.substr(pos + 1));
				m_ScreenShotBottomRight = { x, y };
			}
			return AXQResult::ok;
		}
		std::string cmd;
		// Top left
		std::cout << "Please put the mouse in the top left of chess board, input \"tl\" for sure" << std::endl;
		while (std::cin >> cmd)
		{
			if (cmd == "tl")
				break;
		}
		if (!GetCursorPos(&m_ScreenShotTopLeft))
			return AXQResult::fail;
		std::cout << "Screen shot top left point: " << m_ScreenShotTopLeft.x << ", " << m_ScreenShotTopLeft.y << std::endl;
		// Bottom right
		std::cout << "Please put the mouse in the bottom right of chess board, input \"br\" for sure" << std::endl;
		while (std::cin >> cmd)
		{
			if (cmd == "br")
				break;
		}
		if (!GetCursorPos(&m_ScreenShotBottomRight))
			return AXQResult::fail;
		std::cout << "Screen shot bottom right point: " << m_ScreenShotBottomRight.x << ", " << m_ScreenShotBottomRight.y << std::endl;

		// write setting to file
		std::string line = "m_ScreenShotTopLeft=" + std::to_string(m_ScreenShotTopLeft.x) + "," + std::to_string(m_ScreenShotTopLeft.y);
		m_RW << line << std::endl;
		line = "m_ScreenShotBottomRight=" + std::to_string(m_ScreenShotBottomRight.x) + "," + std::to_string(m_ScreenShotBottomRight.y);
		m_RW << line << std::endl;

		return AXQResult::ok;
	}

	AXQResult AutoChesser::LocateWindow()
	{
		std::string cmd;
		POINT curPoint;
		if (m_ReadSetting)
		{
			std::string value = m_Settings["gameWindowPoint"];
			auto pos = value.find(",");
			if (pos != std::string::npos)
			{
				int x = stoi(value.substr(0, pos));
				int y = stoi(value.substr(pos + 1));
				gameWindow = WindowFromPoint({ x, y });
			}
		}
		else
		{
			// Window
			std::cout << "Please put the mouse in the game window, input \"win\" for sure" << std::endl;
			while (std::cin >> cmd)
			{
				if (cmd == "win")
					break;
			}
			if (!GetCursorPos(&curPoint))
				return AXQResult::fail;
			gameWindow = WindowFromPoint(curPoint);
			std::string line = "gameWindowPoint=" + std::to_string(curPoint.x) + "," + std::to_string(curPoint.y);
			m_RW << line << std::endl;
		}
		
		// Bash
		std::cout << "Please put the mouse in the this bash, input \"bash\" for sure" << std::endl;
		while (std::cin >> cmd)
		{
			if (cmd == "bash")
				break;
		}
		if (!GetCursorPos(&curPoint))
			return AXQResult::fail;
		bashWindow = WindowFromPoint(curPoint);

		return AXQResult::ok;
	}

	AXQResult AutoChesser::Run(IPC& ipc, RunType runType)
	{
		std::string cmd;
		// Run type
		std::cout << "There are three type to run, input the index to choose, default is 1" << std::endl;
		std::cout << "\t1. Move piece by message:" << std::endl;
		std::cout << "\t2. Move piece by mouse" << std::endl;
		std::cout << "\t3. Move piece like human" << std::endl;
		while (std::cin >> cmd)
		{
			if (cmd == "1")
				runType = RunType::MOVE_PIECE_BY_MESSAGE;
			else if (cmd == "2")
				runType = RunType::MOVE_PIECE_BY_MOUSE;
			else if (cmd == "3")
				runType = RunType::MOVE_PIECE_LIKE_HUMAN;
			break;
		}

		std::cout << "Setting is ready" << std::endl;
		std::cout << "Input ngr (new game as red) or ngb (new game as black)" << std::endl;
		std::cout << "Input n (next) or ' (close to Enter) to get best move and move chess piece automatically" << std::endl;

		std::string color = "w";  // default is red
		while (std::cin >> cmd)
		{
			if (cmd == "ngr")
			{
				color = "w";
				ipc.Write("ucinewgame");
			}
			else if (cmd == "ngb")
			{
				color = "b";
				ipc.Write("ucinewgame");
			}
			else if (cmd == "n" || cmd == "'")  // next
			{
				// Scan board to fen
				std::string fen = m_FenGen.GenerateFen();
				if (color == "b")
					std::reverse(fen.begin(), fen.end());
				//fen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR";
				ipc.Write("position fen " + fen + " " + color + " - - 0 1");
				std::cout << ("position fen " + fen + " " + color + " - - 0 1") << std::endl;
				ipc.Write("go depth 15");
				std::string bestMove;
				std::string output;
				while (true)
				{
					DWORD readBytes = 0;
					ipc.Read(engineOutput, BuffSize, readBytes);
					engineOutput[readBytes] = '\0';
					output += engineOutput;
					auto pos = output.find("bestmove");
					if (pos != std::string::npos && (pos + std::string("bestmove xxxx").size() <= output.size()))
					{
						bestMove = output.substr(pos + std::string("bestmove ").size(), std::string("xxxx").size());
						break;
					}
				}
				if (color == "b")
				{
					for (auto& c : bestMove)
					{
						switch (c)
						{
						case 'a':
							c = 'i';
							break;
						case 'b':
							c = 'h';
							break;
						case 'c':
							c = 'g';
							break;
						case 'd':
							c = 'f';
							break;
						case 'e':
							c = 'e';
							break;
						case 'f':
							c = 'd';
							break;
						case 'g':
							c = 'c';
							break;
						case 'h':
							c = 'b';
							break;
						case 'i':
							c = 'a';
							break;
						default:
							c = (9 - (c - '0')) + '0';
							break;
						}
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
				POINT from{ bc[row1][col1].x / dpi + m_ScreenShotTopLeft.x, bc[row1][col1].y / dpi + m_ScreenShotTopLeft.y };
				POINT to{ bc[row1][col2].x / dpi + m_ScreenShotTopLeft.x, bc[row2][col2].y / dpi + m_ScreenShotTopLeft.y };

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
					GetWindowRect(bashWindow, &rect);
					POINT originPos;
					GetCursorPos(&originPos);
					SetCursorPos(rect.left + 20, rect.bottom - 20);
					mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
					SetCursorPos(originPos.x, originPos.y);
				}
			}
		}
		return AXQResult::ok;
	}

	void AutoChesser::MovePieceByMessage(POINT from, POINT to)
	{
		RECT rect;
		GetWindowRect(gameWindow, &rect);
		SendMessage(gameWindow, WM_LBUTTONDOWN, 0, from.x - rect.left + ((from.y - rect.top) << 16));
		SendMessage(gameWindow, WM_LBUTTONUP, 0, from.x - rect.left + ((from.y - rect.top) << 16));
		Sleep(500);
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
	}

	void AutoChesser::MovePieceLikeHuman(POINT from, POINT to)
	{
		ImitateHumanMove(from);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);
		ImitateHumanMove(to);
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