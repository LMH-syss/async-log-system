#pragma once

#include <cstddef>
#include <fstream>
#include <string>

class FileAppender {
public:
    FileAppender() = default;
    FileAppender(const std::string& logDir, const std::string& baseName, std::size_t maxFileSize);
    ~FileAppender();

    bool open(const std::string& logDir, const std::string& baseName, std::size_t maxFileSize);
    bool append(const std::string& line);
    void flush();
    void close();

private:
    std::string buildLogFilePath() const;
    bool openCurrentFile();
    void selectWritableFile();
    bool rotateIfNeeded(std::size_t nextLineSize);
    static bool createDirectory(const std::string& path);
    static bool fileExists(const std::string& path);
    static std::size_t fileSize(const std::string& path);
    static std::string currentDate();

    std::string logDir_;
    std::string baseName_;
    std::string date_;
    std::size_t maxFileSize_{0};
    std::size_t currentSize_{0};
    int fileIndex_{1};
    std::ofstream stream_;
};
