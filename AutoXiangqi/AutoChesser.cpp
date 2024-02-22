#include "AutoChesser.h"

namespace axq
{
	AXQResult AutoChesser::LocateChessBoard()
	{
		std::string cmd;
		// Top left
		std::cout << "Please put the mouse in the top left of chess board, input \"tl\" for sure" << std::endl;
		while (std::cin >> cmd)
		{
			if (cmd == "tl")
				break;
		}
		if (!GetCursorPos(&topLeft))
			return AXQResult::fail;
		std::cout << "topLeft point: " << topLeft.x << ", " << topLeft.y << std::endl;
		// Bottom right
		std::cout << "Please put the mouse in the bottom right of chess board, input \"br\" for sure" << std::endl;
		while (std::cin >> cmd)
		{
			if (cmd == "br")
				break;
		}
		if (!GetCursorPos(&bottomRight))
			return AXQResult::fail;
		std::cout << "bottomRight point: " << bottomRight.x << ", " << bottomRight.y << std::endl;
		
		return AXQResult::ok;
	}

	AXQResult AutoChesser::Run1(bool activeBash)
	{
		std::string cmd;
		// Window
		std::cout << "Please put the mouse in the game window, input \"window\" for sure" << std::endl;
		while (std::cin >> cmd)
		{
			if (cmd == "window")
				break;
		}
		POINT curPoint;
		if (!GetCursorPos(&curPoint))
			return AXQResult::fail;
		gameWindow = WindowFromPoint(curPoint);
		// Bash
		if (activeBash)
		{
			std::cout << "Please put the mouse in the this bash, input \"bash\" for sure" << std::endl;
			while (std::cin >> cmd)
			{
				if (cmd == "bash")
					break;
			}
		}
		return AXQResult::ok;
	}

	AXQResult AutoChesser::Run2(bool activeBash)
	{
		std::string cmd;
		// Bash
		if (activeBash)
		{
			std::cout << "Please put the mouse in the this bash, input \"bash\" for sure" << std::endl;
			while (std::cin >> cmd)
			{
				if (cmd == "bash")
					break;
			}
		}
		return AXQResult::ok;
	}
}