#include "LoggerConfig.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {

std::string trim(const std::string& value) {
    std::string::size_type begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }

    std::string::size_type end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(begin, end - begin);
}

std::string toUpper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

bool parseSize(const std::string& value, std::size_t& result) {
    std::istringstream iss(value);
    unsigned long long parsed = 0;
    iss >> parsed;
    if (!iss || !iss.eof()) {
        return false;
    }
    result = static_cast<std::size_t>(parsed);
    return true;
}

bool parseLevel(const std::string& value, LogLevel& level) {
    const std::string upper = toUpper(trim(value));
    if (upper == "DEBUG") {
        level = LogLevel::DEBUG;
        return true;
    }
    if (upper == "INFO") {
        level = LogLevel::INFO;
        return true;
    }
    if (upper == "WARN" || upper == "WARNING") {
        level = LogLevel::WARN;
        return true;
    }
    if (upper == "ERROR" || upper == "ERR") {
        level = LogLevel::ERR;
        return true;
    }
    if (upper == "FATAL") {
        level = LogLevel::FATAL;
        return true;
    }
    return false;
}

bool parseQueueFullPolicy(const std::string& value, QueueFullPolicy& policy) {
    const std::string upper = toUpper(trim(value));
    if (upper == "BLOCK" || upper == "BLOCK_WAIT" || upper == "BLOCKING") {
        policy = QueueFullPolicy::BLOCK;
        return true;
    }
    if (upper == "DROP_LOW_LEVEL" || upper == "DROP_LOW" || upper == "DROP") {
        policy = QueueFullPolicy::DROP_LOW_LEVEL;
        return true;
    }
    return false;
}

} // namespace

bool loadLoggerConfig(const std::string& configPath, LoggerConfig& config) {
    std::ifstream input(configPath.c_str());
    if (!input.is_open()) {
        std::cerr << "Failed to open logger config: " << configPath << std::endl;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        const std::string::size_type comment = line.find_first_of("#;");
        if (comment != std::string::npos) {
            line = trim(line.substr(0, comment));
        }

        const std::string::size_type equal = line.find('=');
        if (equal == std::string::npos) {
            std::cerr << "Invalid logger config line " << lineNumber << ": " << line << std::endl;
            continue;
        }

        const std::string key = toUpper(trim(line.substr(0, equal)));
        const std::string value = trim(line.substr(equal + 1));

        if (key == "LOG_DIR" || key == "LOGDIR") {
            config.logDir = value;
        } else if (key == "BASE_NAME" || key == "BASENAME" || key == "FILE_PREFIX") {
            config.baseName = value;
        } else if (key == "MAX_FILE_SIZE" || key == "MAXFILESIZE") {
            std::size_t parsed = 0;
            if (parseSize(value, parsed)) {
                config.maxFileSize = parsed;
            }
        } else if (key == "MAX_QUEUE_SIZE" || key == "MAXQUEUESIZE") {
            std::size_t parsed = 0;
            if (parseSize(value, parsed)) {
                config.maxQueueSize = parsed;
            }
        } else if (key == "LEVEL" || key == "LOG_LEVEL" || key == "LOGLEVEL") {
            LogLevel parsed = LogLevel::DEBUG;
            if (parseLevel(value, parsed)) {
                config.level = parsed;
            }
        } else if (key == "QUEUE_FULL_POLICY" || key == "QUEUEFULLPOLICY") {
            QueueFullPolicy parsed = QueueFullPolicy::BLOCK;
            if (parseQueueFullPolicy(value, parsed)) {
                config.queueFullPolicy = parsed;
            }
        }
    }

    return true;
}

const char* toString(QueueFullPolicy policy) {
    switch (policy) {
    case QueueFullPolicy::BLOCK:
        return "BLOCK";
    case QueueFullPolicy::DROP_LOW_LEVEL:
        return "DROP_LOW_LEVEL";
    default:
        return "UNKNOWN";
    }
}

