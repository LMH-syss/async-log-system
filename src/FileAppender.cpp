#include "FileAppender.h"

#include <chrono>
#include <cerrno>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <direct.h>
#endif

FileAppender::FileAppender(const std::string& logDir, const std::string& baseName, std::size_t maxFileSize) {
    open(logDir, baseName, maxFileSize);
}

FileAppender::~FileAppender() {
    close();
}

bool FileAppender::open(const std::string& logDir, const std::string& baseName, std::size_t maxFileSize) {
    close();

    logDir_ = logDir;
    baseName_ = baseName;
    date_ = currentDate();
    maxFileSize_ = maxFileSize;
    fileIndex_ = 1;

    if (!createDirectory(logDir_)) {
        std::cerr << "Failed to create log directory: " << logDir_ << std::endl;
        return false;
    }

    selectWritableFile();
    return openCurrentFile();
}

bool FileAppender::append(const std::string& line) {
    const auto nextLineSize = line.size() + 1;
    if (!rotateIfNeeded(nextLineSize)) {
        return false;
    }

    if (!stream_.is_open()) {
        return false;
    }

    stream_ << line << '\n';
    if (!stream_) {
        std::cerr << "Failed to write log line." << std::endl;
        return false;
    }

    currentSize_ += nextLineSize;
    return true;
}

bool FileAppender::openCurrentFile() {
    const auto filePath = buildLogFilePath();
    stream_.open(filePath.c_str(), std::ios::out | std::ios::app | std::ios::binary);
    if (!stream_.is_open()) {
        std::cerr << "Failed to open log file: " << filePath << std::endl;
        return false;
    }

    currentSize_ = fileSize(filePath);

    return true;
}

void FileAppender::flush() {
    if (stream_.is_open()) {
        stream_.flush();
    }
}

void FileAppender::close() {
    if (stream_.is_open()) {
        stream_.close();
    }
}

std::string FileAppender::buildLogFilePath() const {
    std::ostringstream fileName;
    fileName << baseName_ << '_' << date_ << '_' << std::setw(3) << std::setfill('0') << fileIndex_ << ".log";

    if (logDir_.empty()) {
        return fileName.str();
    }

    const char last = logDir_[logDir_.size() - 1];
    if (last == '/' || last == '\\') {
        return logDir_ + fileName.str();
    }

#if defined(_WIN32)
    return logDir_ + "\\" + fileName.str();
#else
    return logDir_ + "/" + fileName.str();
#endif
}

void FileAppender::selectWritableFile() {
    while (true) {
        const auto filePath = buildLogFilePath();
        if (!fileExists(filePath)) {
            currentSize_ = 0;
            return;
        }

        currentSize_ = fileSize(filePath);

        if (maxFileSize_ == 0 || currentSize_ < maxFileSize_) {
            return;
        }

        ++fileIndex_;
    }
}

bool FileAppender::rotateIfNeeded(std::size_t nextLineSize) {
    const auto today = currentDate();
    const bool dateChanged = today != date_;
    const bool sizeExceeded = maxFileSize_ > 0 && currentSize_ > 0 && currentSize_ + nextLineSize > maxFileSize_;
    if (!dateChanged && !sizeExceeded) {
        return true;
    }

    close();
    if (dateChanged) {
        date_ = today;
        fileIndex_ = 1;
    } else {
        ++fileIndex_;
    }

    selectWritableFile();
    return openCurrentFile();
}

bool FileAppender::createDirectory(const std::string& path) {
    if (path.empty()) {
        return true;
    }

    for (std::size_t i = 1; i <= path.size(); ++i) {
        if (i != path.size() && path[i] != '/' && path[i] != '\\') {
            continue;
        }

        std::string current = path.substr(0, i);
        if (current.empty() || current == "/" || current == "\\") {
            continue;
        }

        if (current.size() == 2 && current[1] == ':') {
            continue;
        }

#if defined(_WIN32)
        if (_mkdir(current.c_str()) != 0 && errno != EEXIST) {
            return false;
        }
#else
        if (mkdir(current.c_str(), 0755) != 0 && errno != EEXIST) {
            return false;
        }
#endif
    }

    return true;
}

bool FileAppender::fileExists(const std::string& path) {
    std::ifstream input(path.c_str(), std::ios::binary);
    return input.good();
}

std::size_t FileAppender::fileSize(const std::string& path) {
    std::ifstream input(path.c_str(), std::ios::binary | std::ios::ate);
    if (!input.is_open()) {
        return 0;
    }

    const std::ifstream::pos_type size = input.tellg();
    if (size <= 0) {
        return 0;
    }

    return static_cast<std::size_t>(size);
}

std::string FileAppender::currentDate() {
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::system_clock::to_time_t(now);

    std::tm timeInfo{};
#if defined(_WIN32)
    localtime_s(&timeInfo, &seconds);
#else
    localtime_r(&seconds, &timeInfo);
#endif

    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%Y%m%d");
    return oss.str();
}
