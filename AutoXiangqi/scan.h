#pragma once
#include <opencv2/opencv.hpp>
#include <Windows.h>

// Show the process
void FindAllPiece();

// Find the top left point and bottom right point
void FindAllPiece(int& top, int& left, int& bottom, int& right, int& radius);

void MakePieceFingerPrint();

void MakePieceFingerPrint(std::unordered_map<std::string, cv::Mat>& PieceID);

cv::Mat GetCaptureScreen(HWND hwnd);

std::string Board2Fen(const cv::Mat& img, std::unordered_map<std::string, cv::Mat>& pieceID);

struct XiangqiPoint
{
    int x;
    int y;
};

struct BoardPointInfo
{
    std::string name = "r";
    int score = 0x0FFFFFFF;
};

class ChessTools
{
public:
    bool GetWindowMouseAt(HWND& window)
    {
        POINT curPoint;
        if (!GetCursorPos(&curPoint))
            return false;

        window = WindowFromPoint(curPoint);
        return true;
    }

    bool MoveXiangqiPiece(POINT begin, POINT end)
    {
        if (!SetCursorPos(begin.x, begin.y))
            return false;
        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        Sleep(500);
        if (!SetCursorPos(end.x, end.y))
            return false;
        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }

    // map coordinate
    // x - 'a'
    // 9 - x

};