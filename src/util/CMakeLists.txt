set(util_SRCS
    append_file.cpp
    current_thread.cpp
    timestamp.cpp
)

add_library(xutil ${util_SRCS})
target_link_libraries(xutil pthread rt)

install(TARGETS xutil DESTINATION lib)      # 将xutil库，安装到 lib 目录

file(GLOB HEADERS "*.h")                    # 将.h 文件赋值给HEADERS变量
install(FILES ${HEADERS} DESTINATION include/xraft/util)    # 将HEADERS变量安装到 include/xraft/util 目录
