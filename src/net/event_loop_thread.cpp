#include "src/net/event_loop_thread.h"

#include "src/net/event_loop.h"

namespace xraft {
namespace net {

EventLoopThread::EventLoopThread()
    : loop_(nullptr), exiting_(false), thread_(), mutex_(), cond_() {

}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    loop_->quit();
    thread_.join();
}

EventLoop* EventLoopThread::start_loop() {
    thread_ = std::thread(&EventLoopThread::thread_func, this);

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
    }

    return loop_;
}

void EventLoopThread::thread_func() {
    EventLoop loop;
    
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
}

}
}