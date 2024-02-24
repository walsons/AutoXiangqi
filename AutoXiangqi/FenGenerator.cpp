#include "FenGenerator.h"
#include <Windows.h>

namespace axq
{
    FenGenerator::FenGenerator(POINT topLeft, POINT bottomRight)
        : photoTopLeft(topLeft), photoBottomRight(bottomRight)
    {
    }

	double FenGenerator::GetWindowDpi()
	{
		HWND hd = GetDesktopWindow();
		int zoom = GetDpiForWindow(hd);
		double dpi = 1;
		switch (zoom) {
		case 96:
			dpi = 1;
			break;
		case 120:
			dpi = 1.25;
			break;
		case 144:
			dpi = 1.5;
			break;
		case 192:
			dpi = 2;
			break;
		default:
			std::cout << "GetWindowDpi() error" << std::endl;
			break;
		}
		return dpi;
	}

	void FenGenerator::BoardScreenShot(cv::Mat& boardScreenShot)
	{
		// Screen shot
		HDC hScreenDC = GetDC(NULL);
		HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
		int dpi = GetWindowDpi();
		int width = GetDeviceCaps(hScreenDC, HORZRES) * dpi;
		int height = GetDeviceCaps(hScreenDC, VERTRES) * dpi;
		HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
		HGDIOBJ oldObject = SelectObject(hMemoryDC, hBitmap);
		int shotWidth = (photoBottomRight.x - photoTopLeft.x) * dpi;
		int shotHeight = (photoBottomRight.y - photoTopLeft.y) * dpi;
        // set cursor to another place in case affect recognition
        POINT originPos;
        GetCursorPos(&originPos);
        SetCursorPos(0, 0);
		BitBlt(hMemoryDC, 0, 0, shotWidth, shotHeight, hScreenDC, dpi * photoTopLeft.x, dpi * photoTopLeft.y, SRCCOPY);
        SetCursorPos(originPos.x, originPos.y);
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
		// Transform to Mat
		cv::Mat screenShot(height, width, CV_8UC3, buf.data());
        // Truncate and change to gray
		cv::Rect boardRect = { 0, 0, shotWidth, shotHeight };
        cv::Mat grayImage;
        cv::cvtColor(screenShot(boardRect), grayImage, cv::COLOR_BGR2GRAY);
		boardScreenShot = grayImage.clone();
	}

	void FenGenerator::MakePieceFingerPrint(cv::Mat boardScreenShot)
	{
        SetBoardCoordinate(boardScreenShot);
        auto& src = boardScreenShot;
        auto& b = boardCoordinate;
        auto& r = pieceRadius;
        cv::Rect rect(0, 0, 0, 0);

        /***** Finger print for red *****/
        // r: 车 [9][0]
        rect = cv::Rect(b[9][0].x - r, b[9][0].y - r, 2 * r, 2 * r);
        pieceID["R"] = src(rect);;
        // n: 马 [9][1]
        rect = cv::Rect(b[9][1].x - r, b[9][1].y - r, 2 * r, 2 * r);
        pieceID["N"] = src(rect);
        // b: 象 [9][2]
        rect = cv::Rect(b[9][2].x - r, b[9][2].y - r, 2 * r, 2 * r);
        pieceID["B"] = src(rect);
        // a: 仕 [9][3]
        rect = cv::Rect(b[9][3].x - r, b[9][3].y - r, 2 * r, 2 * r);
        pieceID["A"] = src(rect);
        // k: 帅 [9][4]
        rect = cv::Rect(b[9][4].x - r, b[9][4].y - r, 2 * r, 2 * r);
        pieceID["K"] = src(rect);
        // c: 炮 [7][1]
        rect = cv::Rect(b[7][1].x - r, b[7][1].y - r, 2 * r, 2 * r);
        pieceID["C"] = src(rect);
        // p: 兵 [6][0]
        rect = cv::Rect(b[6][0].x - r, b[6][0].y - r, 2 * r, 2 * r);
        pieceID["P"] = src(rect);

        /***** Finger print for black *****/
        // r: 车 [0][0]
        rect = cv::Rect(b[0][0].x - r, b[0][0].y - r, 2 * r, 2 * r);
        pieceID["r"] = src(rect);
        // n: 马 [0][1]
        rect = cv::Rect(b[0][1].x - r, b[0][1].y - r, 2 * r, 2 * r);
        pieceID["n"] = src(rect);
        // b: 相 [0][2]
        rect = cv::Rect(b[0][2].x - r, b[0][2].y - r, 2 * r, 2 * r);
        pieceID["b"] = src(rect);
        // a: 士 [0][3]
        rect = cv::Rect(b[0][3].x - r, b[0][3].y - r, 2 * r, 2 * r);
        pieceID["a"] = src(rect);
        // k: 将 [0][4]
        rect = cv::Rect(b[0][4].x - r, b[0][4].y - r, 2 * r, 2 * r);
        pieceID["k"] = src(rect);
        // c: 砲 [2][1]
        rect = cv::Rect(b[2][1].x - r, b[2][1].y - r, 2 * r, 2 * r);
        pieceID["c"] = src(rect);
        // p: 卒 [3][0]
        rect = cv::Rect(b[3][0].x - r, b[3][0].y - r, 2 * r, 2 * r);
        pieceID["p"] = src(rect);

        /***** Finger print for blank *****/
        rect = cv::Rect(b[4][0].x - r, b[4][0].y - r, 2 * r, 2 * r);
        pieceID["blank1"] = src(rect);
        rect = cv::Rect(b[4][2].x - r, b[4][2].y - r, 2 * r, 2 * r);
        pieceID["blank2"] = src(rect);
        rect = cv::Rect(b[4][4].x - r, b[4][4].y - r, 2 * r, 2 * r);
        pieceID["blank3"] = src(rect);
        rect = cv::Rect(b[4][6].x - r, b[4][6].y - r, 2 * r, 2 * r);
        pieceID["blank4"] = src(rect);
        rect = cv::Rect(b[4][8].x - r, b[4][8].y - r, 2 * r, 2 * r);
        pieceID["blank5"] = src(rect);
        rect = cv::Rect(b[5][0].x - r, b[5][0].y - r, 2 * r, 2 * r);
        pieceID["blank6"] = src(rect);
        rect = cv::Rect(b[5][2].x - r, b[5][2].y - r, 2 * r, 2 * r);
        pieceID["blank7"] = src(rect);
        rect = cv::Rect(b[5][4].x - r, b[5][4].y - r, 2 * r, 2 * r);
        pieceID["blank8"] = src(rect);
        rect = cv::Rect(b[5][6].x - r, b[5][6].y - r, 2 * r, 2 * r);
        pieceID["blank9"] = src(rect);
        rect = cv::Rect(b[5][8].x - r, b[5][8].y - r, 2 * r, 2 * r);
        pieceID["blank10"] = src(rect);
	}

    std::string FenGenerator::GenerateFen()
    {
        cv::Mat img;
        BoardScreenShot(img);
        clock_t beginTime = clock();
        std::string fen;
        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                auto& b = boardCoordinate;
                auto rect = cv::Rect(b[i][j].x - pieceRadius, b[i][j].y - pieceRadius, 2 * pieceRadius, 2 * pieceRadius);
                cv::Mat onePiece = img(rect);
                // Compare to all piece
                int minValue = 0x0FFFFFFF;
                std::string target;
                // a method to accelerate
                cv::Mat possiblePiece = pieceID[boardPointInfo[i][j].name];
                int outScore = SimilarityScore(onePiece, possiblePiece);
                int diff = outScore * 0.05;
                if (outScore - diff < boardPointInfo[i][j].score && outScore + diff > boardPointInfo[i][j].score)
                {
                    minValue = outScore;
                    target = boardPointInfo[i][j].name;
                    // targetImg = pieceID[boardPointInfo[i][j].name];
                }
                else
                {
                    for (auto it = pieceID.begin(); it != pieceID.end(); ++it)
                    {
                        int score = SimilarityScore(onePiece, it->second);
                        if (score < minValue)
                        {
                            minValue = score;
                            target = it->first;
                            // targetImg = it->second;
                        }
                    }
                }
                boardPointInfo[i][j].name = target;
                boardPointInfo[i][j].score = minValue;
                /*if (i == 0 && j == 6)
                {
                    imshow("dd", onePiece);
                    waitKey();
                    imshow("dd3", targetImg);
                    waitKey();
                }
                if (i == 0 && j == 7)
                {
                    imshow("dd", onePiece);
                    waitKey();
                    imshow("dd3", targetImg);
                    waitKey();
                }*/
                //std::cout << "minValue: " << minValue << " -----------  " << "target: " << target << std::endl;
                if (target.size() == 1)
                    fen += target;
                else
                {
                    if (fen.empty() || (fen.back() < '0' || fen.back() >= '9'))
                        fen += "1";
                    else
                        fen.back() = fen.back() + 1;
                }
            }
            fen += "/";
        }
        fen.pop_back();
        clock_t endTime = clock();
        std::cout << "time cost: " << (endTime - beginTime) << std::endl;
        return fen;
    }

    void FenGenerator::SetBoardCoordinate(cv::Mat boardScreenShot)
    {
        std::vector<cv::Vec3f> circles;
        int minDist = 30;   // minimum distance
        int param1 = 100;     // Canny
        int param2 = 15;     // Hough
        int minRadius = 30;   // min radius
        int maxRadius = 35;   // max radius
        cv::HoughCircles(boardScreenShot, circles, cv::HOUGH_GRADIENT, 1, minDist, param1, param2, minRadius, maxRadius);
        for (size_t i = 0; i < circles.size(); i++)
        {
            boardTop = boardTop < circles[i][1] ? boardTop : circles[i][1];
            boardLeft = boardLeft < circles[i][0] ? boardLeft : circles[i][0];
            boardBottom = boardBottom > circles[i][1] ? boardBottom : circles[i][1];
            boardRight = boardRight > circles[i][0] ? boardRight : circles[i][0];
            pieceRadius += circles[i][2];
        }
        pieceRadius /= circles.size();

        int hStep = (boardRight - boardLeft) / 8;
        int vStep = (boardBottom - boardTop) / 9;
        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                boardCoordinate[i][j] = POINT{ boardLeft + j * hStep, boardTop + i * vStep };
            }
        }
    }

    int FenGenerator::SimilarityScore(cv::Mat img1, cv::Mat img2)
    {
        cv::Mat ig1 = img1(cv::Range(img1.rows / 4, img1.rows * 3 / 4), cv::Range(img1.cols / 4, img1.cols * 3 / 4));
        int interval = 5;
        int row = ig1.rows / interval + ((ig1.rows % interval) > 0 ? 1 : 0);
        int col = ig1.cols / interval + ((ig1.rows % interval) > 0 ? 1 : 0);
        std::vector<std::vector<int>> arr(row, std::vector<int>(col, 0));
        for (int i = 0; i < row; ++i)
        {
            for (int j = 0; j < col; ++j)
            {
                arr[i][j] = ig1.at<uchar>(interval * i, interval * j);
            }
        }
        ///////////////////////////////////
        int minScore = 0x0FFFFFFF;
        for (int i = 0; i < img2.rows - ig1.rows + 1; ++i)
        {
            for (int j = 0; j < img2.cols - ig1.cols + 1; ++j)
            {
                /**********/
                cv::Mat ig2 = img2(cv::Range(i, i + ig1.rows), cv::Range(j, j + ig1.cols));
                int score = 0;
                for (int x = 0; x < row; ++x)
                {
                    for (int y = 0; y < col; ++y)
                    {
                        score += abs(arr[x][y] - ig2.at<uchar>(interval * x, interval * y));
                    }
                }
                minScore = minScore < score ? minScore : score;
                /**********/
            }
        }
        return minScore;
    }
}