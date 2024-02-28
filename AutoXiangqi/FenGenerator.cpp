#include "FenGenerator.h"
#include <Windows.h>
#include <wingdi.h>

namespace axq
{
    static COLORREF redColor = RGB(255, 0, 0);
    static COLORREF blueColor = RGB(0, 0, 255);
    static COLORREF greenColor = RGB(0, 255, 0);

    long calY(long x, double k, double b)
    {
        return static_cast<long>(k * x + b);
    }
    long calX(long y, double k, double b)
    {
        return static_cast<long>((y - b) / k);
    }
    void drawLine(HDC hdc, POINT p1, POINT p2)
    {
        if (p1.x == p2.x)
        {
            long beg = p1.y, end = p2.y;
            if (beg > end)
                std::swap(beg, end);
            for (long i = beg; i <= end; ++i)
            {
                SetPixel(hdc, p1.x, i, blueColor);
            }
        }
        double k = (static_cast<double>(p1.y) - p2.y) / (static_cast<double>(p1.x) - p2.x);
        double b = p1.y - k * p1.x;
        auto draw = [&](long beg, long end, long (*f)(long, double, double)) {
            if (beg > end)
                std::swap(beg, end);
            for (long i = beg; i <= end; ++i)
            {
                if (f == &calY)
                    SetPixel(hdc, i, f(i, k, b), blueColor);
                else
                    SetPixel(hdc, f(i, k, b), i, blueColor);
            }
        };
        if (std::abs(p1.x - p2.x) > std::abs(p1.y - p2.y))
            draw(p1.x, p2.x, &calY);
        else
            draw(p1.y, p2.y, &calX);
    }

    std::atomic<bool> drawThreadRunning = true;

    void drawBoard(POINT tl, POINT tr, POINT br, POINT bl)
    {
        HDC hScreenDC = GetDC(NULL);
        while (drawThreadRunning)
        {
            drawLine(hScreenDC, tl, tr);
            drawLine(hScreenDC, tr, br);
            drawLine(hScreenDC, br, bl);
            drawLine(hScreenDC, bl, tl);
        }
        ReleaseDC(NULL, hScreenDC);
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
		int shotWidth = (m_ScreenShotBottomRight.x - m_ScreenShotTopLeft.x) * dpi;
		int shotHeight = (m_ScreenShotBottomRight.y - m_ScreenShotTopLeft.y) * dpi;
		BitBlt(hMemoryDC, 0, 0, shotWidth, shotHeight, hScreenDC, dpi * m_ScreenShotTopLeft.x, dpi * m_ScreenShotTopLeft.y, SRCCOPY);
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

    void FenGenerator::GameTimerShot(cv::Mat& gameTimerShot)
    {
        gameTimerShot = SnippingGray(NULL, m_GameTimerTopLeft, m_GameTimerBottomRight).clone();
    }

	void FenGenerator::MakePieceFingerPrint(cv::Mat boardScreenShot)
	{
        SetBoardCoordinate(boardScreenShot);
        auto& src = boardScreenShot;
        auto& b = boardCoordinate;
        auto& r = pieceRadius;
        cv::Rect rect(0, 0, 0, 0);

        /*std::cout << (boardCoordinate[0][0].x + photoTopLeft.x * 2) << ", " << (boardCoordinate[0][0].y + photoTopLeft.y * 2) << std::endl;
        std::cout << (boardCoordinate[0][8].x + photoTopLeft.x * 2) << ", " << (boardCoordinate[0][8].y + photoTopLeft.y * 2) << std::endl;
        std::cout << (boardCoordinate[9][8].x + photoTopLeft.x * 2) << ", " << (boardCoordinate[9][8].y + photoTopLeft.y * 2) << std::endl;
        std::cout << (boardCoordinate[9][0].x + photoTopLeft.x * 2) << ", " << (boardCoordinate[9][0].y + photoTopLeft.y * 2) << std::endl;
        drawBoard(
            { boardCoordinate[0][0].x + photoTopLeft.x * 2, boardCoordinate[0][0].y + photoTopLeft.y * 2 },
            { boardCoordinate[0][8].x + photoTopLeft.x * 2, boardCoordinate[0][8].y + photoTopLeft.y * 2 },
            { boardCoordinate[9][8].x + photoTopLeft.x * 2, boardCoordinate[9][8].y + photoTopLeft.y * 2 },
            { boardCoordinate[9][0].x + photoTopLeft.x * 2, boardCoordinate[9][0].y + photoTopLeft.y * 2 });*/

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

    bool FenGenerator::IsMyTurn()
    {
        cv::Mat img;
        GameTimerShot(img);
        cv::Mat origin = cv::imread("gameTimer.png", 0);
        int outScore = SimilarityScore(img, origin);
        assert(img.rows == origin.rows);
        assert(img.cols == origin.cols);
        int score = 0;
        for (int i = 0; i < img.rows; ++i)
        {
            for (int j = 0; j < img.cols; ++j)
            {
                score += std::abs(img.at<uchar>(i, j) - origin.at<uchar>(i, j));
            }
        }
        if (score > 5 * img.rows * img.cols)
            return true;
        return false;
    }

    std::string FenGenerator::GenerateFen()
    {
        cv::Mat img;
        BoardScreenShot(img);
        clock_t beginTime = clock();
        std::string fen;
        std::unordered_map<char, short> m;
        int color = 0;
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
                        }
                    }
                }
                boardPointInfo[i][j].name = target;
                boardPointInfo[i][j].score = minValue;
                int selfColor = 0;
                bool valid = IsValidCharInFen((target.size() == 1 ? target[0] : 'e'), i, j, m, selfColor);
                if (!valid)
                    return "";
                if (selfColor != 0)
                {
                    if (color == -selfColor)
                        return "";
                    color = selfColor;
                }

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
        std::cout << "I'm " << color << " (red is 1, black is -1)" << std::endl;
        if (color == -1)
            std::reverse(fen.begin(), fen.end());
        // "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR";
        fen += (std::string(" ") + (color == 1 ? "w" : "b") + " - - 0 1");
        return fen;
    }

    cv::Mat FenGenerator::SnippingGray(HWND win, POINT tl, POINT br)
    {
        // Screen shot
        HDC hdc = GetDC(win);
        HDC hMemoryDC = CreateCompatibleDC(hdc);
        int dpi = GetWindowDpi();
        int width = GetDeviceCaps(hdc, HORZRES) * dpi;
        int height = GetDeviceCaps(hdc, VERTRES) * dpi;
        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
        HGDIOBJ oldObject = SelectObject(hMemoryDC, hBitmap);
        int shotWidth = (br.x - tl.x) * dpi;
        int shotHeight = (br.y - tl.y) * dpi;
        BitBlt(hMemoryDC, 0, 0, shotWidth, shotHeight, hdc, dpi * tl.x, dpi * tl.y, SRCCOPY);
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
        return grayImage.clone();
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

    bool FenGenerator::IsValidCharInFen(char key, int x, int y, std::unordered_map<char, short>& m, int& selfColor)
    {
        selfColor = 0;
        ++m[key];
        switch (key)
        {
        case 'r':
        case 'n':
        case 'c':
        case 'R':
        case 'N':
        case 'C':
        {
            if (m[key] < 0 || m[key] > 2)
                return false;
            return true;
        }
        break;
        case 'b':
        case 'B':
        {
            if (m[key] < 0 || m[key] > 2)
                return false;
            // position: (0,2) (0,6) (2,0) (2,4) (2,8) (4,2) (4,6) --> top
            constexpr const int corb[7][2] = { {0,2}, {0,6}, {2,0}, {2,4}, {2,8}, {4,2}, {4,6} };
            // position: (9,2) (9,6) (7,0) (7,4) (7,8) (5,2) (5,6) --> bottom
            constexpr const int corB[7][2] = { {9,2}, {9,6}, {7,0}, {7,4}, {7,8}, {5,2}, {5,6} };
            for (size_t i = 0; i < sizeof(corb) / sizeof(corb[0]); ++i)
            {
                if (x == corb[i][0] && y == corb[i][1])
                {
                    selfColor = (key == 'b' ? 1 : -1);
                    return true;
                }
            }
            for (size_t i = 0; i < sizeof(corB) / sizeof(corB[0]); ++i)
            {
                if (x == corB[i][0] && y == corB[i][1])
                {
                    selfColor = (key == 'B' ? 1 : -1);
                    return true;
                }
            }
            return false;
        }
        break;
        case 'a':
        case 'A':
        {
            if (m[key] < 0 || m[key] > 2)
                return false;
            // position: (0,3) (0,5) (1,4) (2,3) (2,5) --> top
            constexpr const int cora[5][2] = { {0,3}, {0,5}, {1,4}, {2,3}, {2,5} };
            // position: (9,3) (9,5) (8,4) (7,3) (7,5) --> bottom
            constexpr const int corA[5][2] = { {9,3}, {9,5}, {8,4}, {7,3}, {7,5} };
            for (size_t i = 0; i < sizeof(cora) / sizeof(cora[0]); ++i)
            {
                if (x == cora[i][0] && y == cora[i][1])
                {
                    selfColor = (key == 'a' ? 1 : -1);
                    return true;
                }
            }
            for (size_t i = 0; i < sizeof(corA) / sizeof(corA[0]); ++i)
            {
                if (x == corA[i][0] && y == corA[i][1])
                {
                    selfColor = (key == 'A' ? 1 : -1);
                    return true;
                }
            }
            return false;
        }
        break;
        case 'k':
        case 'K':
        {
            if (m[key] != 1)
                return false;
            // position 0 <= x <= 2, 3 <= y <= 5 --> top
            // position 7 <= x <= 9, 3 <= y <= 5 --> bottom
            if (y >= 3 && y <= 5)
            {
                if (x >= 0 && x <= 2)
                {
                    selfColor = (key == 'k' ? 1 : -1);
                    return true;
                }
                else if (x >= 7 && x <= 9)
                {
                    selfColor = (key == 'K' ? 1 : -1);
                    return true;
                }
            }
            return false;
        }
        break;
        case 'p':
        case 'P':
        {
            if (m[key] < 0 || m[key] > 5)
                return false;
            // x >= 3 --> top
            // x <= 6 --> bottom
            if (x > 6)
            {
                selfColor = (key == 'p' ? 1 : -1);
            }
            else if (x < 3)
            {
                selfColor = (key == 'P' ? 1 : -1);
            }
            return true;
        }
        break;
        case 'e': // empty/blank
        {
            return true;
        }
        break;
        default:
            break;
        }
        return false;
    }
}