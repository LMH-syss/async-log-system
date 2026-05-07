#include "Logger.h"

#include <iostream>

int main() {
    Logger::instance().init("test_logs", "test_async_log", 1024);
    Logger::instance().setLevel(LogLevel::WARN);

    LOG_DEBUG("this debug message should be filtered");
    LOG_WARN("this warning message should be written");
    LOG_ERROR("this error message should be written");

    Logger::instance().shutdown();

    std::cout << "async_log_test finished." << std::endl;
    return 0;
}

