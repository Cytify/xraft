set(log_SRCS
    async_log.cpp
    log_file.cpp
    log_stream.cpp
    logger.cpp
)

add_library(xlog ${log_SRCS})
target_link_libraries(xlog xcon xutil)

install(TARGETS xlog DESTINATION lib)

set(HEADERS
    async_log.h
    log_file.h
    log_stream.h
    logger.h
)
install(FILES ${HEADERS} DESTINATION include/xraft/log)

add_subdirectory(test)