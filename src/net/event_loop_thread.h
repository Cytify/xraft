#ifndef EVENT_LOOP_THREAD_H_
#define EVENT_LOOP_THREAD_H_

#include <mutex>
#include <condition_variable>
#include <thread>

#include "src/util/noncopyable.h"

namespace xraft {
namespace net {

class EventLoop;

class EventLoopThread : util::Noncopyable {
public:
    EventLoopThread();

    ~EventLoopThread();

    EventLoop* start_loop();

private:
    void thread_func();

    EventLoop* loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

}
}

#endif