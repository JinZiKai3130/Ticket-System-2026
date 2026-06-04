cmake_minimum_required(VERSION 3.10)
project(TicketManagementSystem)

# 设置 C++ 标准（根据需要修改，例如 11 或 17）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 收集 src 目录下所有 .cpp 文件（包括 main.cpp 和其他源文件）
file(GLOB SOURCES "src/*.cpp")

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 指定头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE include)

# 如果有额外的编译选项（可选）
# target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)