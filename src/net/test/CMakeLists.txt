add_executable(basic_reactor_test test_basic_reactor.cpp)
target_link_libraries(basic_reactor_test xnet xlog xutil)

add_executable(run_in_loop_test test_run_in_loop.cpp)
target_link_libraries(run_in_loop_test xnet xlog xutil)

add_executable(event_loop_thread_test test_event_loop_thread.cpp)
target_link_libraries(event_loop_thread_test xnet xlog xutil)

add_executable(acceptor_test test_acceptor.cpp)
target_link_libraries(acceptor_test xnet xlog xutil)

add_executable(tcp_server_test test_tcp_server.cpp)
target_link_libraries(tcp_server_test xnet xlog xutil)