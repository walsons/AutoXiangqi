#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

namespace axq
{
    std::wstring string2wstring(const std::string& str);

    std::string wstring2string(const std::wstring& str);

    bool IsIdenticalImage(cv::Mat image1, cv::Mat image2, int threshold);
}

#endif
