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
		FenGenerator(POINT topLeft, POINT bottomRight);
		double GetWindowDpi();
		void BoardScreenShot(cv::Mat& boardScreenShot);
		void MakePieceFingerPrint(cv::Mat boardScreenShot);
		std::string GenerateFen();

	private:
		void SetBoardCoordinate(cv::Mat boardScreenShot);
		int SimilarityScore(cv::Mat img1, cv::Mat img2);

	public:
		POINT photoTopLeft = { 0, 0 };
		POINT photoBottomRight = { 0, 0 };
		POINT boardCoordinate[10][9];
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
