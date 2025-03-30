#pragma once

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>

class Log
{
public:
    inline static bool m_printStubs = true;

    template <typename... Args>
    static void Info(const std::string &prefix, Args &&...args)
    {
        std::stringstream ss;

        ss << "[" << GetCurrentTime() << "]"
           << "[Info]"
           << "[" << prefix << "]: ";

        AppendToStream(ss, std::forward<Args>(args)...);

        std::cout << ss.str() << std::endl;
    }

    template <typename... Args>
    static void Stub(const std::string &prefix, Args &&...args)
    {
        if (m_printStubs)
        {
            std::stringstream ss;

            ss << "[" << GetCurrentTime() << "]"
               << "[STUB]"
               << "[" << prefix << "]: ";

            AppendToStream(ss, std::forward<Args>(args)...);

            std::cout << ss.str() << std::endl;
        }
    }

    template <typename... Args>
    static void Error(const std::string &prefix, Args &&...args)
    {
        std::stringstream ss;

        ss << "[" << GetCurrentTime() << "]"
           << "[Error]"
           << "[" << prefix << "]: ";

        AppendToStream(ss, std::forward<Args>(args)...);

        std::cerr << ss.str() << std::endl;
    }

private:
    static std::string GetCurrentTime()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        tm localTime;

        localtime_s(&localTime, &time);

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << localTime.tm_hour << ":"
           << std::setfill('0') << std::setw(2) << localTime.tm_min;

        return ss.str();
    }

    template <typename T>
    static void AppendToStream(std::stringstream &ss, T &&value)
    {
        ss << std::forward<T>(value);
    }

    template <typename T, typename... Args>
    static void AppendToStream(std::stringstream &ss, T &&value, Args &&...args)
    {
        ss << std::forward<T>(value);
        AppendToStream(ss, std::forward<Args>(args)...);
    }
};