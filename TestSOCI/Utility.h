#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include "spdlog/spdlog.h"
#if WIN32
#include <objbase.h>
#endif

class TickTick
{
public:
    TickTick() :
        t1(std::chrono::steady_clock::now()), tn(t1)
    {
    }
    ~TickTick()
    {
        auto t2 = std::chrono::steady_clock::now();
        spdlog::info("{} milliseconds", std::chrono::duration_cast<std::chrono::milliseconds>(t2 - tn).count());
    }
    void tick()
    {
        auto t2 = std::chrono::steady_clock::now();
        spdlog::info("{} milliseconds", std::chrono::duration_cast<std::chrono::milliseconds>(t2 - tn).count());
        tn = t2;
    }
private:
    const std::chrono::steady_clock::time_point t1;
    std::chrono::steady_clock::time_point tn;
};


static std::string uuid()
{
#if WIN32
    GUID guid;
    if (S_OK == ::CoCreateGuid(&guid))
    {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0')
            << std::setw(8) << guid.Data1 << "-"
            << std::setw(4) << guid.Data2 << "-"
            << std::setw(4) << guid.Data3 << "-"
            // hex 不适用于 char 类型，需要 +
            << std::setw(2) << +guid.Data4[0] << std::setw(2) << +guid.Data4[1] << "-"
            << std::setw(2) << +guid.Data4[2] << std::setw(2) << +guid.Data4[3] << std::setw(2) << +guid.Data4[4]
            << std::setw(2) << +guid.Data4[5] << std::setw(2) << +guid.Data4[6] << std::setw(2) << +guid.Data4[7]
            << std::nouppercase;
        return oss.str();

        char buf[64] = { 0 };
        _snprintf_s(buf, sizeof(buf)
            , "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"
            , guid.Data1
            , guid.Data2
            , guid.Data3
            , guid.Data4[0], guid.Data4[1]
            , guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
            , guid.Data4[6], guid.Data4[7]
        );
        return buf;
    }
#else
    static_assert(false, "Not implemented in platform other than windows");
#endif
    return std::string();
}
