cmake_minimum_required(VERSION 3.10)

project(AsyncLogSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(async_log_system
    src/FileAppender.cpp
    src/Logger.cpp
)

target_include_directories(async_log_system
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(async_log_example
    examples/main.cpp
)

target_link_libraries(async_log_example
    PRIVATE
        async_log_system
)

add_executable(async_log_test
    tests/test_logger.cpp
)

target_link_libraries(async_log_test
    PRIVATE
        async_log_system
)
