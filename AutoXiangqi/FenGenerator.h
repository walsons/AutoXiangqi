#ifndef FEN_GENERATOR_H
#define FEN_GENERATOR_H

#include "Fen.h"

#include <opencv2/opencv.hpp>
#include <Windows.h>

namespace axq
{
	struct BoardPointInfo
	{
		std::string name = "r";
		cv::Mat img;
	};

	class FenGenerator
	{
	public:
		FenGenerator() = default;
		double GetWindowDpi();
        void SnippingChessBoard(cv::Mat& chessBoardShot, HWND gameWindow);
		void BoardScreenShot(cv::Mat& boardScreenShot);
		void GameTimerShot(cv::Mat& gameTimerShot);
		void WaitAnimationOver();
		void MakePieceFingerPrint(cv::Mat boardScreenShot);
		void MakeNewBlankPieceFingerPrint();
		bool IsMyTurn();
		bool IsNewGame(cv::Mat img);
		std::string GenerateFen();
		void CalibrateBoard(bool state);

	private:
		cv::Mat SnippingGray(HWND win, POINT tl, POINT br);
		void SetBoardCoordinate(cv::Mat boardScreenShot);
		void SetBoardCoordinateV2(cv::Mat boardScreenShot);
		int SimilarityScore(cv::Mat img1, cv::Mat img2);
		// selfColor: red is 1, black is -1, unknown is 0
		bool IsValidCharInFen(char key, int x, int y, std::unordered_map<char, short>& m, int& selfColor);
		void drawBoard(POINT tl, POINT tr, POINT br, POINT bl);

	public:
		POINT m_ScreenShotTopLeft = { 0, 0 };
		POINT m_ScreenShotBottomRight = { 0, 0 };
		POINT boardCoordinate[10][9];
		POINT m_GameTimerTopLeft = { 0, 0 };
		POINT m_GameTimerBottomRight = { 0, 0 };
		long boardTop = 0x0FFFFFFF;
		long boardLeft = 0x0FFFFFFF;
		long boardBottom = 0;
		long boardRight = 0;
		long pieceRadius = 0;
		Fen m_InputFen = Fen();

	private:
		BoardPointInfo boardPointInfo[10][9];
		std::unordered_map<std::string, cv::Mat> pieceID;
		std::mutex m_Lock;
		std::atomic<bool> m_DrawThreadRunning = false;
	};
}

#endif 
