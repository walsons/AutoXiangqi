#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <stdlib.h>

namespace axq
{
    std::wstring string2wstring(const std::string& str);

    std::string wstring2string(const std::wstring& str);
}

#endif
