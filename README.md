# AsyncLogSystem

AsyncLogSystem 是一个基于 C++14 实现的轻量级异步日志系统，用于展示多线程编程、生产者-消费者模型、阻塞队列、异步文件写入、日志滚动和 RAII 资源管理能力。

## 技术栈

- C++14
- CMake
- STL
- `std::thread`
- `std::mutex`
- `std::condition_variable`
- `std::atomic`
- `std::ofstream`
- `std::chrono`
- Windows 使用 `_mkdir` 创建目录，Linux/macOS 使用 `mkdir` 创建目录

本项目不依赖第三方库，不依赖 Qt、Boost、数据库或网络库。
当前版本不使用 `std::filesystem`，便于接入仍使用 C++14 的项目。

## 项目结构

```text
AsyncLogSystem/
├─ include/
│  ├─ LogLevel.h
│  ├─ LogMessage.h
│  ├─ BlockingQueue.h
│  ├─ FileAppender.h
│  └─ Logger.h
├─ src/
│  ├─ FileAppender.cpp
│  └─ Logger.cpp
├─ examples/
│  └─ main.cpp
├─ tests/
│  └─ test_logger.cpp
├─ README.md
├─ CMakeLists.txt
└─ .gitignore
```

## 核心功能

- 支持 `DEBUG`、`INFO`、`WARN`、`ERROR`、`FATAL` 五种日志等级。
- 支持最低日志等级过滤。
- 支持多线程并发写日志。
- 业务线程只负责提交日志消息，不直接写磁盘。
- 后台线程异步消费队列并写入文件。
- 支持阻塞队列 `pop` 和安全 `stop`。
- 支持 `shutdown` 安全退出，尽量写完队列中已有日志。
- 支持日志文件按大小滚动。
- 支持日志宏自动记录源文件名和行号。

## 设计思路

整体流程采用生产者-消费者模型：

```text
业务线程调用 LOG_INFO
    ↓
Logger::log 封装 LogMessage
    ↓
LogMessage 进入 BlockingQueue
    ↓
后台日志线程被唤醒
    ↓
后台线程取出日志并格式化
    ↓
FileAppender 写入日志文件
    ↓
文件达到大小限制后滚动到新文件
```

各模块职责保持单一：

- `LogLevel`：定义日志等级并提供字符串转换。
- `LogMessage`：保存一条日志的时间、等级、线程 ID、源文件、行号和正文，并负责格式化输出。
- `BlockingQueue<T>`：基于 `mutex` 和 `condition_variable` 实现线程安全阻塞队列。
- `FileAppender`：负责日志目录创建、文件打开、追加写入、flush、close 和文件滚动，路径通过字符串拼接完成。
- `Logger`：对外提供单例接口，负责等级过滤、消息入队、后台线程和关闭流程。

## 编译运行

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

运行示例程序：

```bash
./async_log_example
```

Windows 下可执行文件通常位于 `build` 目录或生成器对应的配置子目录中，例如：

```powershell
.\async_log_example.exe
```

运行后会在 `logs/` 目录下生成日志文件。

## 使用示例

```cpp
#include "Logger.h"

int main() {
    Logger::instance().init("logs", "async_log", 1024 * 1024);
    Logger::instance().setLevel(LogLevel::DEBUG);

    LOG_INFO("logger started");
    LOG_WARN("something should be checked");
    LOG_ERROR("something failed");

    Logger::instance().shutdown();
    return 0;
}
```

## 日志输出示例

```text
[2026-05-07 19:40:59.115] [INFO] [thread:1] [main.cpp:12] logger started
[2026-05-07 19:40:59.116] [INFO] [thread:3] [main.cpp:18] worker 0 writes log 0
[2026-05-07 19:40:59.116] [WARN] [thread:1] [main.cpp:27] all worker threads finished
```

日志文件命名示例：

```text
async_log_20260507_001.log
async_log_20260507_002.log
async_log_20260507_003.log
```
