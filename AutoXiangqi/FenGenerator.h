#ifndef FEN_GENERATOR_H
#define FEN_GENERATOR_H

#include <opencv2/opencv.hpp>
#include <Windows.h>

namespace axq
{
	struct BoardPointInfo
	{
		std::string name = "r";
		int score = 0x0FFFFFFF;
	};

	class FenGenerator
	{
	public:
		FenGenerator() = default;
		double GetWindowDpi();
		void BoardScreenShot(cv::Mat& boardScreenShot);
		void GameTimerShot(cv::Mat& gameTimerShot);
		void MakePieceFingerPrint(cv::Mat boardScreenShot);
		bool IsMyTurn();
		std::string GenerateFen();

	private:
		cv::Mat SnippingGray(HWND win, POINT tl, POINT br);
		void SetBoardCoordinate(cv::Mat boardScreenShot);
		int SimilarityScore(cv::Mat img1, cv::Mat img2);

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

	private:
		BoardPointInfo boardPointInfo[10][9];
		std::unordered_map<std::string, cv::Mat> pieceID;
	};
}

#endif 
