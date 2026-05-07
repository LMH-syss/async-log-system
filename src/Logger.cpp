#include "Logger.h"

#include <chrono>
#include <utility>

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::init(const std::string& logDir, const std::string& baseName, std::size_t maxFileSize) {
    std::lock_guard<std::mutex> lock(lifecycleMutex_);
    if (initialized_.load() || shutdownCalled_.load()) {
        return;
    }

    auto appender = std::make_unique<FileAppender>();
    if (!appender->open(logDir, baseName, maxFileSize)) {
        return;
    }

    appender_ = std::move(appender);
    initialized_.store(true);
    worker_ = std::thread(&Logger::workerLoop, this);
}

void Logger::setLevel(LogLevel level) {
    level_.store(static_cast<int>(level));
}

void Logger::log(LogLevel level, const std::string& message, const char* file, int line) {
    if (!initialized_.load()) {
        return;
    }

    if (static_cast<int>(level) < level_.load()) {
        return;
    }

    LogMessage logMessage{
        std::chrono::system_clock::now(),
        level,
        std::this_thread::get_id(),
        file ? file : "",
        line,
        message,
    };

    queue_.push(std::move(logMessage));
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(lifecycleMutex_);
    if (!initialized_.load() && !worker_.joinable()) {
        return;
    }

    initialized_.store(false);
    shutdownCalled_.store(true);
    queue_.stop();

    if (worker_.joinable()) {
        worker_.join();
    }

    if (appender_) {
        appender_->flush();
        appender_->close();
        appender_.reset();
    }
}

Logger::~Logger() {
    shutdown();
}

void Logger::workerLoop() {
    LogMessage message;
    while (queue_.pop(message)) {
        if (appender_) {
            appender_->append(message.format());
        }
    }

    if (appender_) {
        appender_->flush();
    }
}
