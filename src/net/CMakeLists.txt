set(net_SRCS
    channel.cpp
    event_loop.cpp
    poller.cpp
)

add_library(xnet ${net_SRCS})
target_link_libraries(xnet xlog xutil)

install(TARGETS xnet DESTINATION lib)

set(HEADERS
    channel.h
    event_loop.h
    poller.h
)
install(FILES ${HEADERS} DESTINATION include/xraft/net)

add_subdirectory(test)