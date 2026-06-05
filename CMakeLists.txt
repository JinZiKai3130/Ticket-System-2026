cmake_minimum_required(VERSION 3.10)
project(TicketManagementSystem)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 将可执行文件输出到项目根目录（即 CMakeLists.txt 所在目录）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 收集 src 目录下所有 .cpp 文件
file(GLOB SOURCES "src/*.cpp")

# 添加可执行文件，名称固定为 code
add_executable(code ${SOURCES})

# 指定头文件目录
target_include_directories(code PRIVATE include)

# 可选：开启常见编译警告
# target_compile_options(code PRIVATE -Wall -Wextra)