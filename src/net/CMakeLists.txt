set(net_SRCS
    channel.cpp
    event_loop.cpp
    poller.cpp
    timer.cpp
    timer_queue.cpp
    event_loop_thread.cpp
    acceptor.cpp
    inet_address.cpp
    socket_ops.cpp
    socket.cpp
)

add_library(xnet ${net_SRCS})
target_link_libraries(xnet xlog xutil)

install(TARGETS xnet DESTINATION lib)

set(HEADERS
    channel.h
    event_loop.h
    poller.h
    timer_id.h
    timer.h
    timer_queue.h
    event_loop_thread.h
    acceptor.h
    inet_address.h
    socket_ops.h
    socket.h
)
install(FILES ${HEADERS} DESTINATION include/xraft/net)

add_subdirectory(test)