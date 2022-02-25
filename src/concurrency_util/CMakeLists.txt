set(con_SRCS
    count_down_latch.cpp
    thread_pool.cpp
)

add_library(xcon ${con_SRCS})
target_link_libraries(xcon xutil)

install(TARGETS xcon DESTINATION lib)

set(HEADERS
    blocking_queue.h
    count_down_latch.h
    thread_pool.h
)
install(FILES ${HEADERS} DESTINATION include/xraft/concurrency_util)

add_subdirectory(test)