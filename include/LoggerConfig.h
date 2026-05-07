#pragma once

#include "LogLevel.h"

#include <cstddef>
#include <string>

enum class QueueFullPolicy {
    BLOCK,
    DROP_LOW_LEVEL
};

struct LoggerConfig {
    std::string logDir = "logs";
    std::string baseName = "async_log";
    std::size_t maxFileSize = 1024 * 1024;
    std::size_t maxQueueSize = 8192;
    LogLevel level = LogLevel::DEBUG;
    QueueFullPolicy queueFullPolicy = QueueFullPolicy::BLOCK;
};

bool loadLoggerConfig(const std::string& configPath, LoggerConfig& config);
const char* toString(QueueFullPolicy policy);

