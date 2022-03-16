set(raft_SRCS
    raft_server.cpp
)

add_library(xraft ${raft_SRCS})
target_link_libraries(xraft xnet)

install(TARGETS xraft DESTINATION lib)

set(HEADERS
    raft_server.h
)
install(FILES ${HEADERS} DESTINATION include/xraft/raft_core)

add_subdirectory(test)