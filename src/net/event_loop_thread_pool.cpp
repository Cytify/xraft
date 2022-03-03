#include "src/net/event_loop_thread_pool.h"

#include <assert.h>
#include <stdio.h>


#include "src/net/event_loop.h"
#include "src/net/event_loop_thread.h"

namespace xraft {
namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, const std::string& name)
    : base_loop_(base_loop), name_(name), started_(false), thread_num_(0), next_(0) {

}

EventLoopThreadPool::~EventLoopThreadPool() {

}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    assert(!started_);
    base_loop_->assert_in_loop_thread();

    started_ = true;

    for (int i = 0; i < thread_num_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread;
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->start_loop());
    }
}

EventLoop* EventLoopThreadPool::get_next_loop() {
    base_loop_->assert_in_loop_thread();
    EventLoop* loop = base_loop_;

    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size()) {
            next_ = 0;
        }
    }

    return loop;
}

EventLoop* EventLoopThreadPool::get_loop_for_hash(size_t hash_code) {

}

std::vector<EventLoop*> EventLoopThreadPool::get_all_loops() {

}

}
}