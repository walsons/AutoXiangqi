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
}