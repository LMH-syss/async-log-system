#pragma once

#include "BlockingQueue.h"
#include "FileAppender.h"
#include "LogLevel.h"
#include "LogMessage.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class Logger {
public:
    static Logger& instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void init(const std::string& logDir, const std::string& baseName, std::size_t maxFileSize);
    void setLevel(LogLevel level);
    void log(LogLevel level, const std::string& message, const char* file, int line);
    void shutdown();

private:
    Logger() = default;
    ~Logger();

    void workerLoop();

    std::mutex lifecycleMutex_;
    std::atomic<int> level_{static_cast<int>(LogLevel::DEBUG)};
    std::atomic<bool> initialized_{false};
    std::atomic<bool> shutdownCalled_{false};
    BlockingQueue<LogMessage> queue_;
    std::unique_ptr<FileAppender> appender_;
    std::thread worker_;
};

#define LOG_DEBUG(message) Logger::instance().log(LogLevel::DEBUG, (message), __FILE__, __LINE__)
#define LOG_INFO(message) Logger::instance().log(LogLevel::INFO, (message), __FILE__, __LINE__)
#define LOG_WARN(message) Logger::instance().log(LogLevel::WARN, (message), __FILE__, __LINE__)
#define LOG_ERROR(message) Logger::instance().log(LogLevel::ERR, (message), __FILE__, __LINE__)
#define LOG_FATAL(message) Logger::instance().log(LogLevel::FATAL, (message), __FILE__, __LINE__)
