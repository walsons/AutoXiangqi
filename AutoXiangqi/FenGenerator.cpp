#include "FenGenerator.h"
#include "IPC.h"
#include "Tools.h"

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

    void FenGenerator::SnippingChessBoard(cv::Mat& chessBoardShot, HWND gameWindow)
    {
        // Screen shot
        HDC hScreenDC = GetDC(gameWindow);
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
        chessBoardShot = grayImage.clone();
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

    void FenGenerator::WaitAnimationOver()
    {
        cv::Mat last = SnippingGray(NULL, { m_GameTimerTopLeft.x + boardCoordinate[4][3].x, m_GameTimerTopLeft.y + boardCoordinate[4][3].y },
            { m_GameTimerBottomRight.x + boardCoordinate[5][5].x, m_GameTimerBottomRight.y + boardCoordinate[5][5].y });
        int loop = 0;
        while (true)
        {
            Sleep(200);
            cv::Mat next = SnippingGray(NULL, { m_GameTimerTopLeft.x + boardCoordinate[4][3].x, m_GameTimerTopLeft.y + boardCoordinate[4][3].y },
                { m_GameTimerBottomRight.x + boardCoordinate[5][5].x, m_GameTimerBottomRight.y + boardCoordinate[5][5].y });
            if (IsIdenticalImage(last, next, 5 * last.rows * last.cols))
            {
                break;
            }
            if (++loop >= 10)
            {
                std::cout << "Wait animation over for 2s" << std::endl;
                break;
            }
        }
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
        pieceID["R"] = src(rect);
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

        /***** Finger print for empty grid *****/
        auto previous = cv::imread("PreviousPhotoForEmptyGrid.png", 0);
        if (previous.data != nullptr)
            src = previous;
        rect = cv::Rect(b[4][3].x - r, b[4][3].y - r, 2 * r, 2 * r);
        pieceID["e1"] = src(rect);
        rect = cv::Rect(b[4][5].x - r, b[4][5].y - r, 2 * r, 2 * r);
        pieceID["e2"] = src(rect);
        rect = cv::Rect(b[5][0].x - r, b[5][0].y - r, 2 * r, 2 * r);
        pieceID["e3"] = src(rect);
        rect = cv::Rect(b[5][2].x - r, b[5][2].y - r, 2 * r, 2 * r);
        pieceID["e4"] = src(rect);
        rect = cv::Rect(b[5][4].x - r, b[5][4].y - r, 2 * r, 2 * r);
        pieceID["e5"] = src(rect);
        rect = cv::Rect(b[5][6].x - r, b[5][6].y - r, 2 * r, 2 * r);
        pieceID["e6"] = src(rect);
        rect = cv::Rect(b[5][8].x - r, b[5][8].y - r, 2 * r, 2 * r);
        pieceID["e7"] = src(rect);
        rect = cv::Rect(b[8][4].x - r, b[8][4].y - r, 2 * r, 2 * r);
        pieceID["e8"] = src(rect);
        rect = cv::Rect(b[8][2].x - r, b[8][2].y - r, 2 * r, 2 * r);
        pieceID["e9"] = src(rect);
        rect = cv::Rect(b[8][6].x - r, b[8][6].y - r, 2 * r, 2 * r);
        pieceID["e10"] = src(rect);
	}

    void FenGenerator::MakeNewBlankPieceFingerPrint()
    {
        cv::Mat boardScreenShot;
        BoardScreenShot(boardScreenShot);
        cv::imwrite("PreviousPhotoForEmptyGrid.png", boardScreenShot);
        auto& src = boardScreenShot;
        auto& b = boardCoordinate;
        auto& r = pieceRadius;
        cv::Rect rect(0, 0, 0, 0);

        /***** Finger print for empty grid *****/
        rect = cv::Rect(b[4][3].x - r, b[4][3].y - r, 2 * r, 2 * r);
        pieceID["e1"] = src(rect);
        rect = cv::Rect(b[4][5].x - r, b[4][5].y - r, 2 * r, 2 * r);
        pieceID["e2"] = src(rect);
        rect = cv::Rect(b[5][0].x - r, b[5][0].y - r, 2 * r, 2 * r);
        pieceID["e3"] = src(rect);
        rect = cv::Rect(b[5][2].x - r, b[5][2].y - r, 2 * r, 2 * r);
        pieceID["e4"] = src(rect);
        rect = cv::Rect(b[5][4].x - r, b[5][4].y - r, 2 * r, 2 * r);
        pieceID["e5"] = src(rect);
        rect = cv::Rect(b[5][6].x - r, b[5][6].y - r, 2 * r, 2 * r);
        pieceID["e6"] = src(rect);
        rect = cv::Rect(b[5][8].x - r, b[5][8].y - r, 2 * r, 2 * r);
        pieceID["e7"] = src(rect);
        rect = cv::Rect(b[8][4].x - r, b[8][4].y - r, 2 * r, 2 * r);
        pieceID["e8"] = src(rect);
        rect = cv::Rect(b[8][2].x - r, b[8][2].y - r, 2 * r, 2 * r);
        pieceID["e9"] = src(rect);
        rect = cv::Rect(b[8][6].x - r, b[8][6].y - r, 2 * r, 2 * r);
        pieceID["e10"] = src(rect);
    }

    bool FenGenerator::IsMyTurn()
    {
        cv::Mat img;
        GameTimerShot(img);
        cv::Mat origin = cv::imread("GameTimerPhoto.png", 0);
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

    bool FenGenerator::IsNewGame(cv::Mat img)
    {
        //BoardScreenShot(img);
        std::string selfSection;
        auto recognize = [&](int x, int y) -> char {
            int minValue = 0x0FFFFFFF;
            std::string target;
            auto& b = boardCoordinate;
            auto rect = cv::Rect(b[x][y].x - pieceRadius, b[x][y].y - pieceRadius, 2 * pieceRadius, 2 * pieceRadius);
            cv::Mat onePiece = img(rect);
            for (auto it = pieceID.begin(); it != pieceID.end(); ++it)
            {
                // Ignore the empty grid
                if ((it->first)[0] == 'e')
                    continue;
                int score = SimilarityScore(onePiece, it->second);
                if (score < minValue)
                {
                    minValue = score;
                    target = it->first;
                }
            }
            return target[0];
        };
        static const std::vector<POINT> selfCoordinate{
            {6, 0}, { 6, 2 }, { 6, 4 }, { 6, 6 }, { 6, 8 },
            {7, 1}, {7, 7},
            // row 8 is empty
            {9, 0}, {9, 1}, { 9, 2 }, { 9, 3 }, { 9, 4 }, { 9, 5 }, { 9, 6 }, { 9, 7 }, { 9, 8 }
        };
        for (const auto& pos : selfCoordinate)
        {
            selfSection += recognize(pos.x, pos.y);
        }
        /***** If game start, update the empty grid, there are five cases *****/
        // self is red;
        static const std::string start1 = "PPPPPCCRNBAKABNR";
        // self is black;
        static const std::string start2 = "pppppccrnbakabnr";
        // self is black, but enemy's cannon move to our piece place 
        static const std::string start3 = "pppppccrCbakabnr";
        static const std::string start4 = "pppppccrnbakabCr";
        if (selfSection == start1 ||
            selfSection == start2 ||
            selfSection == start3 ||
            selfSection == start4)
        {
            std::cout << "Check a new game start" << std::endl;
            return true;
        }
        return false;
    }

    std::string FenGenerator::GenerateFen()
    {
        clock_t beginTime = clock();
        cv::Mat img;
        BoardScreenShot(img);
        if (IsNewGame(img))
        {
            IPC::GetIPC().Write("ucinewgame");
            m_InputFen = Fen();
            for (int i = 0; i < 10; ++i)
            {
                for (int j = 0; j < 9; ++j)
                {
                    boardPointInfo[i][j].img = cv::Mat();
                }
            }
            MakeNewBlankPieceFingerPrint();
        }
        std::string fen;
        std::unordered_map<char, short> m;
        std::atomic<int> color = 0;
        std::unordered_map<char, std::vector<POINT>> mDebugCor;
        std::unordered_map<char, std::vector<int>> mDebugScore;
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
                    // remove white dot influence
                    int whitePixel = 0;
                    for (int i = 0; i < onePiece.rows; ++i)
                    {
                        for (int j = 0; j < onePiece.cols; ++j)
                        {
                            if (onePiece.at<uchar>(i, j) > 245)
                                whitePixel += 1;
                        }
                    }
                    if (whitePixel > 60)
                    {
                        //std::cout << "whitePixel: " << whitePixel << std::endl;
                        target = "e1";
                    }
                    else
                    {
                        // a method to accelerate, check if pixel change  
                        cv::Mat possiblePiece = boardPointInfo[i][j].img;
                        bool pixelChange = false;
                        auto checkPixelChange = [&]() -> bool {
                            if (possiblePiece.size() == onePiece.size())
                            {
                                //unsigned long long diff = 0;
                                for (int i = 0; i < possiblePiece.rows; ++i)
                                {
                                    for (int j = 0; j < possiblePiece.cols; ++j)
                                    {
                                        //diff += abs(possiblePiece.at<uchar>(i, j) - onePiece.at<uchar>(i, j));
                                        if (possiblePiece.at<uchar>(i, j) != onePiece.at<uchar>(i, j))
                                        {
                                            return true;
                                        }
                                    }
                                }
                                //std::cout << "i:" << i << " j:" << j << "  diff: " << diff << std::endl;
                                //if (diff != 0)
                                //    return true;
                                target = boardPointInfo[i][j].name;
                                return false;
                            }
                            return true;
                        };
                        if (checkPixelChange())
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
                            //std::lock_guard<std::mutex> lock(m_Lock);
                            boardPointInfo[i][j].name = target;
                            boardPointInfo[i][j].img = onePiece;
                        }
                        int selfColor = 0;
                        bool valid;
                        {
                            std::lock_guard<std::mutex> lock(m_Lock);
                            char c = target[0];
                            valid = IsValidCharInFen(c, i, j, m, selfColor);
                            mDebugCor[c].push_back({ i, j });
                            mDebugScore[c].push_back(minValue);
                        }
                        if (!valid)
                        {
                            char c = target[0];
                            std::cout << "recognize error: " << c << std::endl;
                            std::cout << "coordinate: ";
                            for (auto& c : mDebugCor[c])
                            {
                                std::cout << c.x << "," << c.y << "  |  ";
                            }
                            std::cout << std::endl;
                            std::cout << "score: ";
                            for (auto& c : mDebugScore[c])
                            {
                                std::cout << c << "  |  ";
                            }
                            std::cout << std::endl;
                            fenSegment = "";
                            return;
                        }
                        if (selfColor != 0)
                        {
                            if (color == -selfColor)
                            {
                                std::cout << fenSegment << " ---color error--- " << selfColor << " " << color << std::endl;
                                fenSegment = "";
                                return;
                            }
                            //std::cout << "color: " << color << "     selfColor: " << selfColor << std::endl;
                            color = selfColor;
                        }
                    }
                    if (target[0] != 'e')
                        fenSegment += target[0];
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
        //BoardScreenShot(img);
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
        /*for (size_t i = 0; i < circles.size(); i++)
        {
            cv::circle(boardScreenShot, cv::Point(circles[i][0], circles[i][1]), circles[i][2], cv::Scalar(0, 0, 255), 2);
        }
        cv::imwrite("circle.png", boardScreenShot);*/
        // Sort circles by y
        std::sort(circles.begin(), circles.end(), [](const cv::Vec3f& c1, const cv::Vec3f& c2) {
            return c1[1] < c2[1];
        });
        // find a series of circle which y is similar
        std::vector<std::vector<cv::Vec3f>> vec;
        std::vector<cv::Vec3f> innerVec;
        cv::Vec3f init = { 0, 0, 1 }, end = {static_cast<float>(boardScreenShot.cols), static_cast<float>(boardScreenShot.rows), 1};
        circles.push_back(end);
        for (size_t i = 0; i < circles.size(); i++)
        {
            if (abs(circles[i][1] - init[1]) < circles[i][2] * 0.1 && abs(circles[i][2] - init[2]) < circles[i][2] * 0.1)
            {
                innerVec.push_back(init);
            }
            else
            {
                innerVec.push_back(init);
                vec.push_back(innerVec);
                innerVec.clear();
            }
            init = circles[i];
        }
        decltype(vec) vecFilter;
        for (int i = 0; i < vec.size(); ++i)
        {
            // A little of error is allowed
            if (vec[i].size() >= 8)
                vecFilter.push_back(vec[i]);
        }
        std::sort(vecFilter.front().begin(), vecFilter.front().end(), [](const cv::Vec3f& c1, const cv::Vec3f& c2) {
            return c1[0] < c2[0];
        });
        std::sort(vecFilter.back().begin(), vecFilter.back().end(), [](const cv::Vec3f& c1, const cv::Vec3f& c2) {
            return c1[0] < c2[0];
        });
        boardLeft = vecFilter.back().front()[0];
        boardRight = vecFilter.back().back()[0];
        boardBottom = (vecFilter.back().front()[1] + vecFilter.back().back()[1]) / 2;
        boardTop = (vecFilter.front().front()[1] + vecFilter.front().back()[1]) / 2;
        std::cout << boardTop << " " << boardLeft << " " << boardBottom << " " << boardRight << std::endl;
        for (auto it = vecFilter.front().begin(); it != vecFilter.front().end(); ++it)
        {
            pieceRadius += (*it)[2];
        }
        pieceRadius /= vecFilter.front().size();
        std::cout << pieceRadius << std::endl;
        
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
        int interval = 3;
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