#include <Windows.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "scan.h"

/*********************/
#include "IPC.h"
#include "ChessEngine.h"
#include "AutoChesser.hpp"
#include "ChessEngine.h"
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

double GetScale()
{
	HWND hd = GetDesktopWindow();
	int zoom = GetDpiForWindow(hd);
	double dpi = 0;
	switch (zoom) {
	case 96:
		dpi = 1;
		std::cout << "100%" << std::endl;
		break;
	case 120:
		dpi = 1.25;
		std::cout << "125%" << std::endl;
		break;
	case 144:
		dpi = 1.5;
		std::cout << "150%" << std::endl;
		break;
	case 192:
		dpi = 2;
		std::cout << "200%" << std::endl;
		break;
	default:
		std::cout << "error" << std::endl;
		break;
	}
	return dpi;
}

std::string sc(const POINT& topLeft, const POINT& bottomRight, std::unordered_map<std::string, cv::Mat>& pieceID)
{
	HDC hScreenDC = GetDC(NULL);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	int dpi = GetScale();
	int width = GetDeviceCaps(hScreenDC, HORZRES) * dpi;
	int height = GetDeviceCaps(hScreenDC, VERTRES) * dpi;
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	HGDIOBJ oldObject = SelectObject(hMemoryDC, hBitmap);
	int shotWidth = (bottomRight.x - topLeft.x) * dpi;
	int shotHeight = (bottomRight.y - topLeft.y) * dpi;
	BitBlt(hMemoryDC, 0, 0, shotWidth, shotHeight, hScreenDC, dpi * topLeft.x, dpi * topLeft.y, SRCCOPY);
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	std::vector<uchar> buf(width * height * 3);
	GetDIBits(hMemoryDC, hBitmap, 0, height,
		buf.data(), (BITMAPINFO*)&bi,
		DIB_RGB_COLORS);
	// 转换为Mat类型
	cv::Mat img(height, width, CV_8UC3, buf.data());

	return Board2Fen(img, pieceID);
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


	const int BuffSize = 4096;
	char engineOutput[BuffSize + 1];
	DWORD readBytes = 0;
	DWORD writeBytes = 0;

	std::string fen;
	std::string chesser;
	
	POINT topLeft = { 0, 0 };
	POINT bottomRight = { 0, 0 };
	std::string pointCmd;
	std::cout << "Please put the mouse in the top left, input tl to go" << std::endl;
	while (std::cin >> pointCmd)
	{
		if (pointCmd == "tl")
			break;
	}
	if (GetCursorPos(&topLeft))
		std::cout << "topLeft point: " << topLeft.x << ", " << topLeft.y << std::endl;
	std::cout << "Please put the mouse in the bottom right, input br to go" << std::endl;
	while (std::cin >> pointCmd)
	{
		if (pointCmd == "br")
			break;
	}
	if (GetCursorPos(&bottomRight))
		std::cout << "bottomRight point: " << bottomRight.x << ", " << bottomRight.y << std::endl;

	POINT bashPos;
	std::cout << "Please put the mouse in the bash, input bash to go" << std::endl;
	while (std::cin >> pointCmd)
	{
		if (pointCmd == "bash")
			break;
	}
	/*if (GetCursorPos(&bashPos))
		std::cout << "bashPos point: " << bashPos.x << ", " << bashPos.y << std::endl;*/
	ChessTools ct1;
	HWND chessWin;
	ct1.GetWindowMouseAt(chessWin);

	// Screen shot
	HDC hScreenDC = GetDC(NULL);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	int dpi = GetScale();
	int width = GetDeviceCaps(hScreenDC, HORZRES) * dpi;
	int height = GetDeviceCaps(hScreenDC, VERTRES) * dpi;
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	HGDIOBJ oldObject = SelectObject(hMemoryDC, hBitmap);
	int shotWidth = (bottomRight.x - topLeft.x) * dpi;
	int shotHeight = (bottomRight.y - topLeft.y) * dpi;
	BitBlt(hMemoryDC, 0, 0, shotWidth, shotHeight, hScreenDC, dpi * topLeft.x, dpi * topLeft.y, SRCCOPY);
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	std::vector<uchar> buf(width * height * 3);
	GetDIBits(hMemoryDC, hBitmap, 0, height,
		buf.data(), (BITMAPINFO*)&bi,
		DIB_RGB_COLORS);
	// 转换为Mat类型
	cv::Mat img(height, width, CV_8UC3, buf.data());
	cv::imwrite("this.png", img);

	//FindAllPiece();
	std::unordered_map<std::string, cv::Mat> pieceID;
	MakePieceFingerPrint(pieceID);

	std::cout << "Setting is ready" << std::endl;
	std::cout << "Input ngr (new game as red) or ngb (new game as black)" << std::endl;
	std::cout << "Input n (next) to get best move" << std::endl;

	while (std::cin >> pointCmd)
	{
		if (pointCmd == "ngr")
		{
			chesser = "w";
			ipc.Write("ucinewgame");
		}
		if (pointCmd == "ngb")
		{
			chesser = "b";
			ipc.Write("ucinewgame");
		}
		else if (pointCmd == "n")  // next
		{
			// Scan board to fen
			std::string fen = sc(topLeft, bottomRight, pieceID);
			if (chesser == "b")
				std::reverse(fen.begin(), fen.end());
			//fen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR";
			//chesser = "w";
			ipc.Write("position fen " + fen + " " + chesser + " - - 0 1");
			std::cout << ("position fen " + fen + " " + chesser + " - - 0 1") << std::endl;
			ipc.Write("go depth 10");
			std::string bestMove;
			while (true)
			{
				std::string output;
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
			if (chesser == "b")
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

			extern XiangqiPoint g_board[10][9];
			/*for (int i = 0; i < 10; ++i)
			{
				for (int j = 0; j < 9; ++j)
				{
					std::cout << g_board[i][j].x / dpi + topLeft.x << ", " << g_board[i][j].y / dpi + topLeft.y << std::endl;
				}
			}*/
			ChessTools ct;
			int col1 = bestMove[0] - 'a';
			int row1 = '9' - bestMove[1];
			int col2 = bestMove[2] - 'a';
			int row2 = '9' - bestMove[3];
			POINT from{ g_board[row1][col1].x / dpi + topLeft.x, g_board[row1][col1].y / dpi + topLeft.y };
			POINT to{ g_board[row1][col2].x / dpi + topLeft.x, g_board[row2][col2].y / dpi + topLeft.y };
			std::cout << from.x << ", " << from.y << "    " << to.x << ", " << to.y << std::endl;
			RECT rect;
			GetWindowRect(chessWin, &rect);
			// 87 564
			SendMessage(chessWin, WM_LBUTTONDOWN, 0, from.x - rect.left + ((from.y - rect.top) << 16));
			SendMessage(chessWin, WM_LBUTTONUP, 0, from.x - rect.left + ((from.y - rect.top) << 16));
			std::cout << rect.left << ", " << rect.top << std::endl;
			Sleep(500);
			SendMessage(chessWin, WM_LBUTTONDOWN, 0, to.x - rect.left + ((to.y - rect.top) << 16));
			SendMessage(chessWin, WM_LBUTTONUP, 0, to.x - rect.left + ((to.y - rect.top) << 16));
			/*ct.MoveXiangqiPiece(from, to);
			Sleep(500);
			if (!SetCursorPos(bashPos.x, bashPos.y))
				return false;
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);*/

		}
	}
	

	// CloseHandles(pi.hProcess, pi.hThread, ParentWriteNode, ParentReadNode, ChildReadNode, ChildWriteNode);
	//system("pause");
	return 0;
}