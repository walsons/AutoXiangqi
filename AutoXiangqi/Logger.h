#ifndef LOGGER_H
#define LOGGER_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <iomanip>
#include <ctime>
#include <sstream>

class Logger
{
public:
    using Level = spdlog::level::level_enum;
    static void SetLevel(Level log_level)
    {
        spdlog::set_level(log_level);
        m_Logger->set_level(log_level);
    }
    static void FlushOn(Level log_level)
    {
        spdlog::flush_on(log_level);
        m_Logger->flush_on(log_level);
    }
    template <typename... Args>
    static void Log(Args&&... args)
    {
        spdlog::log(args...);
        m_Logger->log(args...);
    }

private:
    static std::string LogFileName()
    {
        auto t = std::time(nullptr);
        tm m;
        localtime_s(&m, &t);

        std::ostringstream oss;
        oss << "logs/axqlog_" << std::put_time(&m, "%Y-%m-%d_%H-%M-%S") << ".txt";
        return oss.str();
    }
    inline static auto m_Logger = spdlog::basic_logger_mt("axq_logger", LogFileName());
};

#endif