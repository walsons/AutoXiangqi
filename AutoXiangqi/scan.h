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