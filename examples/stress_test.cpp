#include "Logger.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

int main() {
    if (!Logger::instance().initFromConfig("logger.conf")) {
        std::cerr << "Failed to initialize logger from logger.conf" << std::endl;
        return 1;
    }

    const int threadCount = 8;
    const int logsPerThread = 10000;

    const auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> workers;
    for (int i = 0; i < threadCount; ++i) {
        workers.emplace_back([i] {
            for (int j = 0; j < logsPerThread; ++j) {
                LOG_INFO("stress thread " + std::to_string(i) + " writes log " + std::to_string(j));
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    LOG_WARN("stress test workers finished");
    Logger::instance().shutdown();

    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Stress test finished: " << threadCount * logsPerThread << " logs in " << elapsed << " ms"
              << std::endl;
    return 0;
}

