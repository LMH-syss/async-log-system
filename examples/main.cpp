#include "Logger.h"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

int main() {
    Logger::instance().init("logs", "async_log", 4 * 1024);
    Logger::instance().setLevel(LogLevel::DEBUG);

    LOG_INFO("logger started");

    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back([i] {
            for (int j = 0; j < 100; ++j) {
                LOG_INFO("worker " + std::to_string(i) + " writes log " + std::to_string(j));
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    LOG_WARN("all worker threads finished");
    Logger::instance().shutdown();

    std::cout << "AsyncLogSystem example finished." << std::endl;
    return 0;
}
