#ifndef ASYNC_LOG_H_
#define ASYNC_LOG_H_

#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "src/util/buffer.h"
#include "src/util/noncopyable.h"
#include "src/concurrency_util/count_down_latch.h"

namespace xlog {

class AsyncLog : util::Noncopyable {
public:
    AsyncLog(const std::string& basename, off_t roll_size, int flush_interval = 3);

    ~AsyncLog();

    void start();

    void stop();

    void append(const char* buf, size_t len);

private:
    void thread_func();

    using Buffer =  util::Buffer<util::kLargeBuffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = std::unique_ptr<Buffer>;

    const int flush_interval_;
    const std::string basename_;
    const off_t roll_size_;

    std::atomic_bool running_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    util::CountDownLatch latch_;
    BufferPtr current_buffer_;
    BufferPtr next_buffer_;
    BufferVector buffers;
};

}


#endif