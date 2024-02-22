#ifndef AXQ_DEFINE_H
#define AXQ_DEFINE_H

namespace axq
{
    enum class AXQResult : char
    {
        ok = 0,
        fail
    };

    static constexpr int BuffSize = 4096;
}

#endif