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

## 项目结构

```text
AsyncLogSystem/
├─ include/
│  ├─ LogLevel.h
│  ├─ LogMessage.h
│  ├─ BlockingQueue.h
│  ├─ FileAppender.h
│  ├─ LoggerConfig.h
│  └─ Logger.h
├─ src/
│  ├─ FileAppender.cpp
│  ├─ LoggerConfig.cpp
│  └─ Logger.cpp
├─ examples/
│  ├─ main.cpp
│  └─ stress_test.cpp
├─ tests/
│  └─ test_logger.cpp
├─ logger.conf
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
- 支持最大队列容量，避免日志生产过快导致内存无限增长。
- 支持队列满策略：阻塞等待或丢弃低等级日志。
- 支持 `shutdown` 安全退出，尽量写完队列中已有日志。
- 支持日志文件按大小滚动。
- 支持通过 `logger.conf` 配置日志目录、文件名前缀、日志等级、文件大小和队列容量。
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
- `BlockingQueue<T>`：基于 `mutex` 和 `condition_variable` 实现线程安全阻塞队列，并支持最大容量。
- `FileAppender`：负责日志目录创建、文件打开、追加写入、flush、close 和文件滚动，路径通过字符串拼接完成。
- `LoggerConfig`：读取简单 `key=value` 配置文件，生成日志系统配置。
- `Logger`：对外提供单例接口，负责等级过滤、队列满策略、消息入队、后台线程和关闭流程。

## 配置文件

项目提供 `logger.conf` 示例：

```ini
log_dir=logs
base_name=async_log
max_file_size=1048576
level=DEBUG
max_queue_size=8192
queue_full_policy=block
```

字段说明：

- `log_dir`：日志输出目录。
- `base_name`：日志文件名前缀。
- `max_file_size`：单个日志文件最大大小，单位为字节。
- `level`：最低输出日志等级，可选 `DEBUG`、`INFO`、`WARN`、`ERROR`、`FATAL`。
- `max_queue_size`：最大队列容量，设置为 `0` 表示不限制。
- `queue_full_policy`：队列满策略，`block` 表示阻塞等待，`drop_low_level` 表示丢弃 `DEBUG` 和 `INFO`，保留 `WARN` 及以上日志。

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
    Logger::instance().initFromConfig("logger.conf");

    LOG_INFO("logger started");
    LOG_WARN("something should be checked");
    LOG_ERROR("something failed");

    Logger::instance().shutdown();
    return 0;
}
```

也可以继续使用代码方式初始化：

```cpp
Logger::instance().init("logs", "async_log", 1024 * 1024);
Logger::instance().setLevel(LogLevel::DEBUG);
```

## 服务端项目接入示例

服务端项目中建议在程序启动时初始化日志系统，在所有业务线程退出后再关闭日志系统：

```cpp
int main() {
    Logger::instance().initFromConfig("logger.conf");
    LOG_INFO("server started");

    // run server event loop

    LOG_INFO("server stopping");
    Logger::instance().shutdown();
    return 0;
}
```

如果通过 Ctrl+C 或信号退出服务，应先停止业务线程、网络线程和线程池，再最后调用 `Logger::shutdown()`，避免关闭日志系统后仍有业务线程继续写日志。

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

## 压力测试

项目提供压力测试示例 `async_log_stress_test`，默认 8 个线程，每个线程写入 10000 条日志：

```bash
cmake --build .
./async_log_stress_test
```

该示例通过 `logger.conf` 初始化日志系统。可以通过调整 `max_queue_size` 和 `queue_full_policy` 观察阻塞等待和低等级日志丢弃策略的行为。
