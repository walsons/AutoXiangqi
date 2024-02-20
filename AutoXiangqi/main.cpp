#include <Windows.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "scan.h"

static constexpr int BuffSize = 4096;

HANDLE ParentWriteNode = nullptr;
HANDLE ParentReadNode = nullptr;
HANDLE ChildReadNode = nullptr;
HANDLE ChildWriteNode = nullptr;

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

int CreateEngineProcess(const std::wstring &engineName, PROCESS_INFORMATION& pi, STARTUPINFO& si)
{
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.hStdError = ChildWriteNode;
	si.hStdOutput = ChildWriteNode;
	si.hStdInput = ChildReadNode;
	si.dwFlags |= STARTF_USESTDHANDLES;

	LPWSTR cmd = (LPWSTR)engineName.c_str();
	auto ret = CreateProcess(NULL,
		cmd,     // command line 
		NULL,    // process security attributes 
		NULL,    // primary thread security attributes 
		TRUE,    // handles are inherited 
		0,       // creation flags 
		NULL,    // use parent's environment 
		NULL,    // use parent's current directory 
		&si,     // STARTUPINFO pointer 
		&pi);    // receives PROCESS_INFORMATION
	if (!ret)
		return ErrorExit("CreateProcess");
	return 0;
}

int Read(char buff[], DWORD size, DWORD& readBytes)
{
	auto ret = ReadFile(ParentReadNode, buff, size, &readBytes, NULL);
	return ret ? 0 : 1;
}

int Write(const std::string& cmd)
{
	DWORD writeBytes;
	auto ret = WriteFile(ParentWriteNode, cmd.c_str(), cmd.size(), &writeBytes, NULL);
	return ret ? 0 : 1;
}

int InitEngine()
{
	char engineOutput[BuffSize + 1];
	DWORD readBytes = 0;

	Read(engineOutput, BuffSize, readBytes);
	engineOutput[readBytes] = '\0';
	std::cout << "Engine Info: " << engineOutput << std::endl;

	Write("uci\n");

	Read(engineOutput, BuffSize, readBytes);
	engineOutput[readBytes] = '\0';
	std::cout << "Engine Info: " << engineOutput << std::endl;

	Write("setoption name Threads value 4\n");
	Write("setoption name EvalFile value C:/Users/shayne/source/repos/AutoXiangqi/x64/Debug/pikafish.nnue\n");
	return 0;
}

//LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch (uMsg)
//	{
//	case WM_CREATE:  // 多用于窗口的初始化
//		// 设置窗口扩展样式为WS_EX_LAYERED
//		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
//		// 设置窗口透明度为50%（0-255，255为完全不透明）
//		SetLayeredWindowAttributes(hwnd, 0, 1, LWA_ALPHA);
//		break;
//	case WM_LBUTTONDOWN:
//		MessageBox(NULL, L"Hello World!", L"Hello Message", MB_OK | MB_ICONINFORMATION);
//
//		std::cout << "down=============" << std::endl;
//		break;
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		return 0;
//
//	case WM_PAINT:
//	{
//		PAINTSTRUCT ps;
//		HDC hdc = BeginPaint(hwnd, &ps);
//
//		// All painting occurs here, between BeginPaint and EndPaint.
//
//		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
//
//		EndPaint(hwnd, &ps);
//	}
//	return 0;
//
//	}
//	return DefWindowProc(hwnd, uMsg, wParam, lParam);
//}



#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0) 

HWND GetWindowHandleByPID(DWORD dwProcessID)
{
	HWND h = GetTopWindow(0);
	while (h)
	{
		DWORD pid = 0;
		DWORD dwTheardId = GetWindowThreadProcessId(h, &pid);
		if (dwTheardId != 0)
		{
			if (pid == dwProcessID/*your process id*/)
			{
				// here h is the handle to the window  
				if (GetTopWindow(h))
				{
					return h;
				}
				// return h;   
			}
		}
		h = ::GetNextWindow(h, GW_HWNDNEXT);
	}
	return NULL;
}

void uf(DWORD pid)
{
	HWND hwndNow = GetWindowHandleByPID(pid);
	while (1)
	{
		POINT pNow = { 0, 0 };
		if (GetCursorPos(&pNow))  // 获取鼠标当前位置
		{
			std::cout << pNow.x << " " << pNow.y << "&&&&&&&&&&" << std::endl;
			HWND hwndPointNow = NULL;
			DWORD  dwProcessID = 0;
			RECT rect;
			hwndPointNow = WindowFromPoint(pNow);  // 获取鼠标所在窗口的句柄
			::GetWindowThreadProcessId(hwndPointNow, &dwProcessID);
			std::cout << dwProcessID << std::endl;
			if (pid == dwProcessID)
			{
				GetWindowRect(hwndPointNow, &rect);
				std::cout << "窗口的句柄:" << (int)hwndNow << std::endl;  // 鼠标所在窗口的句柄
				std::cout << "鼠标所在窗口的句柄:" << (int)hwndPointNow << "ID:" << dwProcessID << std::endl;  // 鼠标所在窗口的句柄
				std::cout << "鼠标所在窗口的top:" << rect.top << std::endl;  // 鼠标所在窗口的句柄
				std::cout << "鼠标所在窗口的bottom:" << rect.bottom << std::endl;  // 鼠标所在窗口的句柄
				std::cout << "鼠标所在窗口的left:" << rect.left << std::endl;  // 鼠标所在窗口的句柄
				std::cout << "鼠标所在窗口的right:" << rect.right << std::endl;  // 鼠标所在窗口的句柄
				std::cout << "鼠标所在窗口的宽:" << rect.right - rect.left << std::endl;  // 鼠标所在窗口的句柄
				std::cout << "鼠标所在窗口的高:" << rect.bottom - rect.top << std::endl;  // 鼠标所在窗口的句柄

				short state = 0;
				if (((state = GetAsyncKeyState(MOUSE_MOVED)) & 0x8000) || (state & 0x01)) {
					system("color 97");
				}
				if (((state = GetAsyncKeyState(MOUSE_EVENT)) & 0x8000) || (state & 0x01)) {
					system("color A7");
				}
				if (((state = GetAsyncKeyState(MOUSE_WHEELED)) & 0x8000) || (state & 0x01)) {
					system("color 17");
				}
			}
			Sleep(1000);
		}
	}
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

//int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
int main()
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	// Set the bInheritHandle flag so pipe handles are inherited.
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&ParentReadNode, &ChildWriteNode, &sa, 0))
		return ErrorExit("CreatePipe");
	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if (!SetHandleInformation(ParentReadNode, HANDLE_FLAG_INHERIT, 0))
		return ErrorExit("SetHandleInformation");
	if (!CreatePipe(&ChildReadNode, &ParentWriteNode, &sa, 0))
		return ErrorExit("CreatePipe");
	// Ensure the write handle to the pipe for STDIN is not inherited.
	if (!SetHandleInformation(ParentWriteNode, HANDLE_FLAG_INHERIT, 0))
		return ErrorExit("SetHandleInformation");

	// Start the engine process
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	if (CreateEngineProcess(L"pikafish_x86-64-vnni256.exe", pi, si))
		return ErrorExit("CreateEngineProcess");

	// Do wirte and read using Read() and Write()
	char engineOutput[BuffSize + 1];
	DWORD readBytes = 0;
	DWORD writeBytes = 0;

	if (InitEngine())
		return ErrorExit("InitEngine");

	// Game start
	Write("ucinewgame\n");
	Write("isready\n");
	Read(engineOutput, BuffSize, readBytes);
	engineOutput[readBytes] = '\0';
	std::string isReady = engineOutput;
	if (isReady.find("readyok") == std::string::npos)
		return ErrorExit("isready: " + isReady);

	std::string fen;
	std::string chesser;
	while (true)
	{
		/*fen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR";
		chesser = "w";
		Write("position fen " + fen + " " + chesser + " - - 0 1\n");
		Write("go depth 10\n");
		std::string bestMove;
		while (true)
		{
			std::string output;
			Read(engineOutput, BuffSize, readBytes);
			engineOutput[readBytes] = '\0';
			output += engineOutput;
			auto pos = output.find("bestmove");
			if (pos != std::string::npos && (pos + std::string("bestmove xxxx").size() <= output.size()))
			{
				bestMove = output.substr(pos + std::string("bestmove ").size(), std::string("xxxx").size());
				break;
			}
		}
		std::cout << "bestMove: " << bestMove << std::endl;*/

		// -------------------------
		break;
	}

	// Write("quit\n");
	
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

	std::cout << "Input \"go\" to start game" << std::endl;
	while (std::cin >> pointCmd)
	{
		if (pointCmd == "go")
			break;
	}

	while (std::cin >> pointCmd)
	{
		if (pointCmd == "ngr")
		{
			chesser = "w";
			Write("ucinewgame\n");
		}
		if (pointCmd == "ngb")
		{
			chesser = "b";
			Write("ucinewgame\n");
		}
		else if (pointCmd == "n")  // next
		{
			// Scan board to fen
			std::string fen = sc(topLeft, bottomRight, pieceID);
			if (chesser == "b")
				std::reverse(fen.begin(), fen.end());
			//fen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR";
			//chesser = "w";
			Write("position fen " + fen + " " + chesser + " - - 0 1\n");
			std::cout << ("position fen " + fen + " " + chesser + " - - 0 1") << std::endl;
			Write("go depth 10\n");
			std::string bestMove;
			while (true)
			{
				std::string output;
				Read(engineOutput, BuffSize, readBytes);
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
				std::cout << "bestMove: " << bestMove << std::endl;
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
		}
	}
	

	CloseHandles(pi.hProcess, pi.hThread, ParentWriteNode, ParentReadNode, ChildReadNode, ChildWriteNode);
	//system("pause");
	return 0;
}