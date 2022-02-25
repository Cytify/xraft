cmake_minimum_required(VERSION 2.8)

project(xraft)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CXX_FLAGS
    -g
    -Wall                       # 显示所有警告信息
    -Wextra                     # 打印额外警告信息
    # -Werror                     # 将警告作为错误处理
    -Wconversion                # 当从较宽类型隐式转换为较短类型时，产生警告
    -Wno-unused-parameter       # 警告未使用的变量
    -Wold-style-cast            # 使用 C风格的类型转换时给出警告
    -Woverloaded-virtual        # 重载虚函数名时给出警告
    -Wpointer-arith             # 在算术表达式中使用函数指针时给出警告
    -Wshadow                    # 当一个局部变量掩盖了另一个局部变量时给出警告
    -Wwrite-strings
    -std=c++11
    -rdynamic
)

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

add_subdirectory(src/concurrency_util)
add_subdirectory(src/log)
add_subdirectory(src/net)
add_subdirectory(src/util)