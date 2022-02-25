add_executable(async_log_test test_async_log.cpp)
target_link_libraries(async_log_test xlog xutil)

add_executable(log_file_test test_log_file.cpp)
target_link_libraries(log_file_test xlog xutil)

add_executable(log_stream_test test_log_stream.cpp)
target_link_libraries(log_stream_test xlog xutil)