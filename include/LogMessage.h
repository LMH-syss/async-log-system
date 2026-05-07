#pragma once

#include "LogLevel.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>

struct LogMessage {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::thread::id threadId;
    std::string file;
    int line;
    std::string message;

    std::string format() const {
        const auto seconds = std::chrono::system_clock::to_time_t(timestamp);
        const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                                timestamp.time_since_epoch()) %
                            1000;

        std::tm timeInfo{};
#if defined(_WIN32)
        localtime_s(&timeInfo, &seconds);
#else
        localtime_r(&seconds, &timeInfo);
#endif

        std::ostringstream oss;
        oss << '[' << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setw(3) << std::setfill('0') << millis.count() << "] "
            << '[' << toString(level) << "] "
            << "[thread:" << threadId << "] "
            << '[' << fileName() << ':' << line << "] "
            << message;
        return oss.str();
    }

private:
    std::string fileName() const {
        const std::string::size_type pos = file.find_last_of("/\\");
        if (pos == std::string::npos) {
            return file;
        }
        return file.substr(pos + 1);
    }
};
