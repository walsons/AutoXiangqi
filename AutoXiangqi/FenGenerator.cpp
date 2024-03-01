#include "FenGenerator.h"
#include <Windows.h>
#include <wingdi.h>
#include <fstream>

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

    

    void FenGenerator::drawBoard(POINT tl, POINT tr, POINT br, POINT bl)
    {
        HDC hScreenDC = GetDC(NULL);
        while (m_DrawThreadRunning)
        {
            drawLine(hScreenDC, tl, tr);
            drawLine(hScreenDC, tr, br);
            drawLine(hScreenDC, br, bl);
            drawLine(hScreenDC, bl, tl);
        }
        ReleaseDC(NULL, hScreenDC);
    }

    void FenGenerator::CalibrateBoard(bool state)
    {
        m_DrawThreadRunning = state;
        if (state)
        {
            m_DrawThreadRunning = true;
            std::fstream reader("boardCorrdinate.txt", std::ios::in);
            std::unordered_map<std::string, int> m;
            std::string line;
            while (std::getline(reader, line))
            {
                size_t pos = line.find("=");
                if (pos != std::string::npos)
                {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    m.insert({ key, stoi(value) });
                }
            }
            auto dpi = GetWindowDpi();
            int top = m["boardTop"] + m_ScreenShotTopLeft.y * dpi;
            int left = m["boardLeft"] + m_ScreenShotTopLeft.x * dpi;
            int bottom = m["boardBottom"] + m_ScreenShotTopLeft.y * dpi;
            int right = m["boardRight"] + m_ScreenShotTopLeft.x * dpi;
            POINT tl{ left, top }, tr{ right, top }, br{ right, bottom }, bl{ left, bottom };
            drawBoard(tl, tr, br, bl);
        }
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
        rect = cv::Rect(b[4][3].x - r, b[4][3].y - r, 2 * r, 2 * r);
        pieceID["blank1"] = src(rect);
        rect = cv::Rect(b[4][5].x - r, b[4][5].y - r, 2 * r, 2 * r);
        pieceID["blank2"] = src(rect);
        rect = cv::Rect(b[5][0].x - r, b[5][0].y - r, 2 * r, 2 * r);
        pieceID["blank3"] = src(rect);
        rect = cv::Rect(b[5][2].x - r, b[5][2].y - r, 2 * r, 2 * r);
        pieceID["blank4"] = src(rect);
        rect = cv::Rect(b[5][4].x - r, b[5][4].y - r, 2 * r, 2 * r);
        pieceID["blank5"] = src(rect);
        rect = cv::Rect(b[5][6].x - r, b[5][6].y - r, 2 * r, 2 * r);
        pieceID["blank6"] = src(rect);
        rect = cv::Rect(b[5][8].x - r, b[5][8].y - r, 2 * r, 2 * r);
        pieceID["blank7"] = src(rect);
        rect = cv::Rect(b[8][4].x - r, b[8][4].y - r, 2 * r, 2 * r);
        pieceID["blank8"] = src(rect);
        rect = cv::Rect(b[8][2].x - r, b[8][2].y - r, 2 * r, 2 * r);
        pieceID["blank9"] = src(rect);
        rect = cv::Rect(b[8][6].x - r, b[8][6].y - r, 2 * r, 2 * r);
        pieceID["blank10"] = src(rect);
	}

    void FenGenerator::MakeNewBlankPieceFingerPrint()
    {
        cv::Mat boardScreenShot;
        BoardScreenShot(boardScreenShot);
        auto& src = boardScreenShot;
        auto& b = boardCoordinate;
        auto& r = pieceRadius;
        cv::Rect rect(0, 0, 0, 0);

        /***** Finger print for blank *****/
        rect = cv::Rect(b[4][3].x - r, b[4][3].y - r, 2 * r, 2 * r);
        pieceID["blank1"] = src(rect);
        rect = cv::Rect(b[4][5].x - r, b[4][5].y - r, 2 * r, 2 * r);
        pieceID["blank2"] = src(rect);
        rect = cv::Rect(b[5][0].x - r, b[5][0].y - r, 2 * r, 2 * r);
        pieceID["blank3"] = src(rect);
        rect = cv::Rect(b[5][2].x - r, b[5][2].y - r, 2 * r, 2 * r);
        pieceID["blank4"] = src(rect);
        rect = cv::Rect(b[5][4].x - r, b[5][4].y - r, 2 * r, 2 * r);
        pieceID["blank5"] = src(rect);
        rect = cv::Rect(b[5][6].x - r, b[5][6].y - r, 2 * r, 2 * r);
        pieceID["blank6"] = src(rect);
        rect = cv::Rect(b[5][8].x - r, b[5][8].y - r, 2 * r, 2 * r);
        pieceID["blank7"] = src(rect);
        rect = cv::Rect(b[8][4].x - r, b[8][4].y - r, 2 * r, 2 * r);
        pieceID["blank8"] = src(rect);
        rect = cv::Rect(b[8][2].x - r, b[8][2].y - r, 2 * r, 2 * r);
        pieceID["blank9"] = src(rect);
        rect = cv::Rect(b[8][6].x - r, b[8][6].y - r, 2 * r, 2 * r);
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
        if (score > 20 * img.rows * img.cols)
            return true;
        return false;
    }

    std::string FenGenerator::GenerateFen()
    {
        clock_t beginTime = clock();
        cv::Mat img;
        BoardScreenShot(img);
        std::string fen;
        std::unordered_map<char, short> m;
        std::atomic<int> color = 0;
        auto recognize = [&](int rowStart, int rowEnd, std::string& fenSegment) {
            for (int i = rowStart; i < rowEnd; ++i)
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
                    if (outScore - 5 <= boardPointInfo[i][j].score && outScore + 5 >= boardPointInfo[i][j].score)
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
                    {
                        std::lock_guard<std::mutex> lock(m_Lock);
                        boardPointInfo[i][j].name = target;
                        boardPointInfo[i][j].score = minValue;
                    }
                    int selfColor = 0;
                    bool valid;
                    {
                        std::lock_guard<std::mutex> lock(m_Lock);
                        valid = IsValidCharInFen((target.size() == 1 ? target[0] : 'e'), i, j, m, selfColor);
                    }
                    if (!valid)
                    {
                        std::cout << "error: " << i << ", " << j << "   " << (target.size() == 1 ? target[0] : 'e')
                            << " " << target  << " " << m[(target.size() == 1 ? target[0] : 'e')] << std::endl;
                        fenSegment = "";
                        return;
                    }
                    if (selfColor != 0)
                    {
                        if (color == -selfColor)
                        {
                            fenSegment = "";
                            return;
                        }
                        color = selfColor;
                    }
                    if (target.size() == 1)
                        fenSegment += target;
                    else
                    {
                        if (fenSegment.empty() || (fenSegment.back() < '0' || fenSegment.back() >= '9'))
                            fenSegment += "1";
                        else
                            fenSegment.back() = fenSegment.back() + 1;
                    }
                }
                fenSegment += "/";
            }
        };
        std::string fen1;
        std::thread t1{recognize, 0, 3, std::ref(fen1)};
        std::string fen3;
        std::thread t3{recognize, 7, 10, std::ref(fen3)};
        // Since JJ Xiangqi has animation in the middle chessboard, snipping a new photo after 1s
        /*clock_t middleTime = clock();
        int diff = middleTime - beginTime;
        if (diff < 1500)
            Sleep(1500 - diff);
        std::cout << "diff: " << diff << std::endl;*/
        BoardScreenShot(img);
        std::string fen2;
        recognize(3, 7, fen2);
        t1.join();
        t3.join();
        if (fen1.empty() || fen2.empty() || fen3.empty() || m['k'] != 1 || m['K'] != 1)
        {
            std::cout << "invalid fen is: " << (fen1 + fen2 + fen3) << std::endl;
            std::cout << "fen1: " << fen1 << " fen2: " << fen2 << "fen3: " << fen3 << std::endl;
            return "";
        }
            
        fen = fen1 + fen2 + fen3;
        fen.pop_back();
        clock_t endTime = clock();
        std::cout << "time cost: " << (endTime - beginTime) << std::endl;
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
        std::cout << boardTop << " " << boardLeft << " " << boardBottom << " " << boardRight << std::endl;
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

    void FenGenerator::SetBoardCoordinateV2(cv::Mat boardScreenShot)
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
            std::fstream reader("boardCorrdinate.txt", std::ios::in);
            std::unordered_map<std::string, int> m;
            std::string line;
            while (std::getline(reader, line))
            {
                size_t pos = line.find("=");
                if (pos != std::string::npos)
                {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    m.insert({ key, stoi(value) });
                }
            }
            boardTop = m["boardTop"];
            boardLeft = m["boardLeft"];
            boardBottom = m["boardBottom"];
            boardRight = m["boardRight"];
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