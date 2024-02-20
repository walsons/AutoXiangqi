#include "scan.h"
#include <vector>
#include <Windows.h>

using namespace cv;

// Show the process
void FindAllPiece()
{
	Mat src = imread("this.png", IMREAD_COLOR);
	imshow("origin", src);
	waitKey();

    Mat grayImage;
    cvtColor(src, grayImage, COLOR_BGR2GRAY);
    //imshow("gray", grayImage);
    //waitKey(0);

	/*Mat blurImage;
	blur(src, blurImage, Size(3, 3));
	imshow("blur", blurImage);
	waitKey();

	Mat edgeImage;
	Canny(blurImage, edgeImage, 50, 100);
	imshow("Canny", edgeImage);
	waitKey();*/
    
    clock_t t1 = clock();
    // 使用 HoughCircles 检测圆
    std::vector<Vec3f> circles;
    // 设置Hough变换参数
    int minDist = 30;   // 最小距离
    int param1 = 100;     // Canny边缘检测高阈值  # 保持和和前面Canny的阈值一致
    int param2 = 15;     // Hough变换高阈值
    int minRadius = 30;   // 最小半径
    int maxRadius = 35;   // 最大半径
    HoughCircles(grayImage, circles, HOUGH_GRADIENT, 1, minDist, param1, param2, minRadius, maxRadius);
    clock_t t2 = clock();
    // 在原图上绘制检测到的圆
    for (size_t i = 0; i < circles.size(); i++)
    {
        std::cout << circles[i][0] << ", " << circles[i][1] << std::endl;
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        circle(src, center, 3, Scalar(0, 255, 0), -1, 8, 0); // 绘制圆心
        circle(src, center, radius, Scalar(0, 0, 255), 2, 8, 0); //绘制空心圆
    }
    rectangle(src, Rect(21, 714, 66, 66), Scalar(0, 0, 255), 2, 8, 0);
    clock_t t3 = clock();
    double timeForHoughCircle = (double)(t2 - t1) / CLOCKS_PER_SEC * 1000;
    double timeForDrawCircle = (double)(t3 - t2) / CLOCKS_PER_SEC * 1000;
    std::cout << "HoughCircles:" << timeForHoughCircle << "ms, DrawCircle:" << timeForDrawCircle << "ms" << std::endl;

    // 显示结果
    imshow("霍夫圆检测", src);
    waitKey(0);
}

// Find the top left point and bottom right point
void FindAllPiece(int& top, int& left, int& bottom, int& right, int& radius)
{
    Mat src = imread("this.png", IMREAD_COLOR);
    Mat grayImage;
    cvtColor(src, grayImage, COLOR_BGR2GRAY);

    clock_t t1 = clock();
    std::vector<Vec3f> circles;
    int minDist = 30;   // minimum distance
    int param1 = 100;     // Canny
    int param2 = 15;     // Hough
    int minRadius = 30;   // min radius
    int maxRadius = 35;   // max radius
    HoughCircles(grayImage, circles, HOUGH_GRADIENT, 1, minDist, param1, param2, minRadius, maxRadius);
    top = 0x0FFFFFFF; left = 0x0FFFFFFF; bottom = 0; right = 0; radius = 0;
    for (size_t i = 0; i < circles.size(); i++)
    {
        top = top < circles[i][1] ? top : circles[i][1];
        left = left < circles[i][0] ? left : circles[i][0];
        bottom = bottom > circles[i][1] ? bottom : circles[i][1];
        right = right > circles[i][0] ? right : circles[i][0];
        radius += circles[i][2];
    }
    radius /= circles.size();
}

struct XiangqiPoint
{
    int x;
    int y;
};

int g_Top = 0, g_Left = 0, g_Bottom = 0, g_Right = 0;
int g_Radius = 0;

void SetCorrdinate(XiangqiPoint board[10][9], int& radius)
{
    //int top = 0, left = 0, bottom = 0, right = 0;
    radius = 0;
    FindAllPiece(g_Top, g_Left, g_Bottom, g_Right, radius);
    g_Radius = radius;
    int hStep = (g_Right - g_Left) / 8;
    int vStep = (g_Bottom - g_Top) / 9;
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 9; ++j)
        {
            board[i][j] = XiangqiPoint{ g_Left + j * hStep, g_Top + i * vStep };
        }
    }
}

// r: 车 
// n: 马
// b: 象
// a: 仕
// k: 帅
// c: 炮
// p: 兵
/*----- assume red is myself -----*/
void MakePieceFingerPrint()
{
    Mat src = imread("this.png", IMREAD_COLOR);
    XiangqiPoint b[10][9];
    int r = 0;
    SetCorrdinate(b, r);
    Rect rect(0, 0, 0, 0);

    /***** Finger print for red *****/
    // r: 车 [9][0]
    rect = Rect(b[9][0].x - r, b[9][0].y - r, 2 * r, 2 * r);
    Mat rRedMat = src(rect);
    imshow("rRed", rRedMat);
    waitKey();
    // n: 马 [9][1]
    rect = Rect(b[9][1].x - r, b[9][1].y - r, 2 * r, 2 * r);
    Mat nRedMat = src(rect);
    imshow("nRed", nRedMat);
    waitKey();
    // b: 象 [9][2]
    rect = Rect(b[9][2].x - r, b[9][2].y - r, 2 * r, 2 * r);
    Mat bRedMat = src(rect);
    imshow("bRed", bRedMat);
    waitKey();
    // a: 仕 [9][3]
    rect = Rect(b[9][3].x - r, b[9][3].y - r, 2 * r, 2 * r);
    Mat aRedMat = src(rect);
    imshow("aRed", aRedMat);
    waitKey();
    // k: 帅 [9][4]
    rect = Rect(b[9][4].x - r, b[9][4].y - r, 2 * r, 2 * r);
    Mat kRedMat = src(rect);
    imshow("kRed", kRedMat);
    waitKey();
    // c: 炮 [7][1]
    rect = Rect(b[7][1].x - r, b[7][1].y - r, 2 * r, 2 * r);
    Mat cRedMat = src(rect);
    imshow("cRed", cRedMat);
    waitKey();
    // p: 兵 [6][0]
    rect = Rect(b[6][0].x - r, b[6][0].y - r, 2 * r, 2 * r);
    Mat pRedMat = src(rect);
    imshow("pRed", pRedMat);
    waitKey();
    
    /***** Finger print for black *****/
    // r: 车 [0][0]
    rect = Rect(b[0][0].x - r, b[0][0].y - r, 2 * r, 2 * r);
    Mat rBlackMat = src(rect);
    imshow("rBlack", rBlackMat);
    waitKey();
    // n: 马 [0][1]
    rect = Rect(b[0][1].x - r, b[0][1].y - r, 2 * r, 2 * r);
    Mat nBlackMat = src(rect);
    imshow("nBlack", nBlackMat);
    waitKey();
    // b: 相 [0][2]
    rect = Rect(b[0][2].x - r, b[0][2].y - r, 2 * r, 2 * r);
    Mat bBlackMat = src(rect);
    imshow("bBlack", bBlackMat);
    waitKey();
    // a: 士 [0][3]
    rect = Rect(b[0][3].x - r, b[0][3].y - r, 2 * r, 2 * r);
    Mat aBlackMat = src(rect);
    imshow("aBlack", aBlackMat);
    waitKey();
    // k: 将 [0][4]
    rect = Rect(b[0][4].x - r, b[0][4].y - r, 2 * r, 2 * r);
    Mat kBlackMat = src(rect);
    imshow("kBlack", kBlackMat);
    waitKey();
    // c: 砲 [2][1]
    rect = Rect(b[2][1].x - r, b[2][1].y - r, 2 * r, 2 * r);
    Mat cBlackMat = src(rect);
    imshow("cBlack", cBlackMat);
    waitKey();
    // p: 卒 [4][0]
    rect = Rect(b[4][0].x - r, b[4][0].y - r, 2 * r, 2 * r);
    Mat pBlackMat = src(rect);
    imshow("pBlack", pBlackMat);
    waitKey();
}

void MakePieceFingerPrint(std::unordered_map<std::string, Mat>& PieceID)
{
    Mat src = imread("this.png", IMREAD_COLOR);
    XiangqiPoint b[10][9];
    int r = 0;
    SetCorrdinate(b, r);
    Rect rect(0, 0, 0, 0);

    /***** Finger print for red *****/
    // r: 车 [9][0]
    rect = Rect(b[9][0].x - r, b[9][0].y - r, 2 * r, 2 * r);
    PieceID["R"] = src(rect);;
    // n: 马 [9][1]
    rect = Rect(b[9][1].x - r, b[9][1].y - r, 2 * r, 2 * r);
    PieceID["N"] = src(rect);
    // b: 象 [9][2]
    rect = Rect(b[9][2].x - r, b[9][2].y - r, 2 * r, 2 * r);
    PieceID["B"] = src(rect);
    // a: 仕 [9][3]
    rect = Rect(b[9][3].x - r, b[9][3].y - r, 2 * r, 2 * r);
    PieceID["A"] = src(rect);
    // k: 帅 [9][4]
    rect = Rect(b[9][4].x - r, b[9][4].y - r, 2 * r, 2 * r);
    PieceID["K"] = src(rect);
    // c: 炮 [7][1]
    rect = Rect(b[7][1].x - r, b[7][1].y - r, 2 * r, 2 * r);
    PieceID["C"] = src(rect);
    // p: 兵 [6][0]
    rect = Rect(b[6][0].x - r, b[6][0].y - r, 2 * r, 2 * r);
    PieceID["P"] = src(rect);

    /***** Finger print for black *****/
    // r: 车 [0][0]
    rect = Rect(b[0][0].x - r, b[0][0].y - r, 2 * r, 2 * r);
    PieceID["r"] = src(rect);
    // n: 马 [0][1]
    rect = Rect(b[0][1].x - r, b[0][1].y - r, 2 * r, 2 * r);
    PieceID["n"] = src(rect);
    // b: 相 [0][2]
    rect = Rect(b[0][2].x - r, b[0][2].y - r, 2 * r, 2 * r);
    PieceID["b"] = src(rect);
    // a: 士 [0][3]
    rect = Rect(b[0][3].x - r, b[0][3].y - r, 2 * r, 2 * r);
    PieceID["a"] = src(rect);
    // k: 将 [0][4]
    rect = Rect(b[0][4].x - r, b[0][4].y - r, 2 * r, 2 * r);
    PieceID["k"] = src(rect);
    // c: 砲 [2][1]
    rect = Rect(b[2][1].x - r, b[2][1].y - r, 2 * r, 2 * r);
    PieceID["c"] = src(rect);
    // p: 卒 [3][0]
    rect = Rect(b[3][0].x - r, b[3][0].y - r, 2 * r, 2 * r);
    PieceID["p"] = src(rect);

    /***** Finger print for blank *****/
    rect = Rect(b[4][0].x - r, b[4][0].y - r, 2 * r, 2 * r);
    PieceID["blank1"] = src(rect);
    rect = Rect(b[4][2].x - r, b[4][2].y - r, 2 * r, 2 * r);
    PieceID["blank2"] = src(rect);
    rect = Rect(b[4][4].x - r, b[4][4].y - r, 2 * r, 2 * r);
    PieceID["blank3"] = src(rect);
    rect = Rect(b[4][6].x - r, b[4][6].y - r, 2 * r, 2 * r);
    PieceID["blank4"] = src(rect);
    rect = Rect(b[4][8].x - r, b[4][8].y - r, 2 * r, 2 * r);
    PieceID["blank5"] = src(rect);
    rect = Rect(b[5][0].x - r, b[5][0].y - r, 2 * r, 2 * r);
    PieceID["blank6"] = src(rect);
    rect = Rect(b[5][2].x - r, b[5][2].y - r, 2 * r, 2 * r);
    PieceID["blank7"] = src(rect);
    rect = Rect(b[5][4].x - r, b[5][4].y - r, 2 * r, 2 * r);
    PieceID["blank8"] = src(rect);
    rect = Rect(b[5][6].x - r, b[5][6].y - r, 2 * r, 2 * r);
    PieceID["blank9"] = src(rect);
    rect = Rect(b[5][8].x - r, b[5][8].y - r, 2 * r, 2 * r);
    PieceID["blank10"] = src(rect);
}

BITMAPINFOHEADER createBitmapHeader(int width, int height)
{
    BITMAPINFOHEADER  bi;

    // create a bitmap
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    return bi;
}

Mat GetCaptureScreen(HWND hwnd)
{
    Mat src;

    // get handles to a device context (DC)
    HDC hwindowDC = GetDC(hwnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    // define scale, height and width
    int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    std::cout << width << " " << height << "----===" << std::endl;

    // create mat object
    src.create(height, width, CV_8UC4);

    // create a bitmap
    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);

    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);  //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);            //copy from hwindowCompatibleDC to hbwindow

    // avoid memory leak
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return src;
}

int similarityScore(const Mat& img1, const Mat& img2 )
{
    int nl = img1.rows < img2.rows ? img1.rows : img2.rows;
    int nc = img1.cols < img2.cols ? (img1.cols * img1.channels()) : (img2.cols * img2.channels());
    assert(img1.channels() == img2.channels());
    int numChannel = img1.channels();

    // horizontal score
    std::vector<int> h1(nl, 0), h2(nl, 0);
    // vertical score
    std::vector<int> v1(nc / numChannel, 0), v2(nc / numChannel, 0);
    for (int i = 0; i < nl; ++i)
    {
        const uchar* data1 = img1.ptr<uchar>(i);
        const uchar* data2 = img2.ptr<uchar>(i);
        for (int j = 0; j < nc; j += numChannel)
        {
            int val1 = 0, val2 = 0;
            for (int x = 0; x < numChannel; ++x)
            {
                val1 += data1[j + x];
                val2 += data2[j + x];
            }
            h1[i] += val1 / numChannel;
            h2[i] += val2 / numChannel;
            v1[j / numChannel] += val1 / numChannel;
            v2[j / numChannel] += val2 / numChannel;
        }
    }

    const int k = 3;
    int minHDistance = 0x0FFFFFFF;
    int minVDistance = 0x0FFFFFFF;
    for (int i = 0; i < 2 * k + 1; ++i)
    {
        int hScore = 0;
        for (int j = 0; j < h1.size() - 2 * k; ++j)
        {
            hScore += abs(h1[k + j] - h2[i + j]);
        }
        minHDistance = minHDistance < hScore ? minHDistance : hScore;
        int vScore = 0;
        for (int j = 0; j < v1.size() - 2 * k; ++j)
        {
            vScore += abs(v1[k + j] - v2[i + j]);
        }
        minVDistance = minVDistance < vScore ? minVDistance : vScore;
    }

    return minHDistance + minVDistance;
}

int similarityScoreV2(const Mat& img1, const Mat& img2)
{
    Mat img_gray1;
    cvtColor(img1, img_gray1, COLOR_RGB2GRAY);
    //img_gray1 = img_gray1(Range(img_gray1.rows / 5, img_gray1.rows * 4 / 5), Range(img_gray1.cols / 5, img_gray1.cols * 4 / 5));
    Mat img_gray2;
    cvtColor(img2, img_gray2, COLOR_RGB2GRAY);
    //img_gray2 = img_gray2(Range(img_gray2.rows / 5, img_gray2.rows * 4 / 5), Range(img_gray2.cols / 5, img_gray2.cols * 4 / 5));

    Mat ig1 = img_gray1(Range(img_gray1.rows / 4, img_gray1.rows * 3 / 4), Range(img_gray1.cols / 4, img_gray1.cols * 3 / 4));
    //std::cout << "ig1.size()" << ig1.size() << std::endl;
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
    ////////////////
    int minScore = 0x0FFFFFFF;
    for (int i = 0; i < img_gray2.rows - ig1.rows + 1; ++i)
    {
        for (int j = 0; j < img_gray2.cols - ig1.cols + 1; ++j)
        {
            /*****/
            Mat ig2 = img_gray2(Range(i, i + ig1.rows), Range(j, j + ig1.cols));
            int score = 0;
            for (int x = 0; x < row; ++x)
            {
                for (int y = 0; y < col; ++y)
                {
                    score += abs(arr[x][y] - ig2.at<uchar>(interval * x, interval * y));
                }
            }
            minScore = minScore < score ? minScore : score;
            /*****/
        }
    }
    return minScore;
}

std::string Board2Fen(const Mat& img, std::unordered_map<std::string, Mat>& pieceID)
{
    std::string fen;
    XiangqiPoint b[10][9];
    int hStep = (g_Right - g_Left) / 8;
    int vStep = (g_Bottom - g_Top) / 9;
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 9; ++j)
        {
            b[i][j] = XiangqiPoint{ g_Left + j * hStep, g_Top + i * vStep };
            auto rect = Rect(b[i][j].x - g_Radius, b[i][j].y - g_Radius, 2 * g_Radius, 2 * g_Radius);
            Mat onePiece = img(rect);
            // Compare to all piece
            int minValue = 0x0FFFFFFF;
            std::string target;
            Mat targetImg;
            for (auto it = pieceID.begin(); it != pieceID.end(); ++it)
            {
                int score = similarityScoreV2(onePiece, it->second);
                if (score < minValue)
                {
                    minValue = score;
                    target = it->first;
                    targetImg = it->second;
                }
            }
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
    return fen;
}



//#include <Windows.h>
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_MOUSEMOVE:
    {
        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(hwnd, &mousePos);
        //保存鼠标相对于窗口的客户区坐标系中的坐标
        int mouseX = mousePos.x;
        int mouseY = mousePos.y;
    }
    break;
    //其他消息处理代码
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}