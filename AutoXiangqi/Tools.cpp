#include "Tools.h"

namespace axq
{
    std::wstring string2wstring(const std::string& str)
    {
        wchar_t* wstr = new wchar_t[str.size() + 1];
        size_t pReturnValue = 0;
        mbstowcs_s(&pReturnValue, wstr, str.size() + 1, str.c_str(), str.size());
        wstr[str.size()] = L'\0';
        std::wstring ret{ wstr };
        delete[] wstr;
        return ret;
    }

    std::string wstring2string(const std::wstring& str)
    {
        return std::string(str.begin(), str.end());
    }

    bool IsIdenticalImage(cv::Mat image1, cv::Mat image2, int threshold)
    {
        if (image1.size() == image2.size())
        {
            unsigned long long diff = 0;
            for (int i = 0; i < image1.rows; ++i)
            {
                for (int j = 0; j < image1.cols; ++j)
                {
                    diff += abs(image1.at<uchar>(i, j) - image2.at<uchar>(i, j));
                }
                if (diff > threshold)
                    return false;
            }
            return true;
        }
        else
        {
            std::cout << "Two images have different size" << std::endl;
        }
        return false;
    }
}