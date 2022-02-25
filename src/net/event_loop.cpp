#include "event_loop.h"

#include <assert.h>

#include "src/util/current_thread.h"
#include "src/log/async_log.h"
#include "src/log/logger.h"
#include "channel.h"
#include "poller.h"

namespace xraft {
namespace net {

// 标记当前线程的event loop 对象
__thread EventLoop* t_loop_in_this_thread = nullptr;

const int kPollTimeMs = 10000;

EventLoop::EventLoop() : 
    looping_(false), thread_id_(util::current_thread::get_tid()),
    quit_(false), poller_(new Poller(this)) {
    if (t_loop_in_this_thread) {
        LOG_ERROR << "EventLoop already exists in this thread, thread id: " << thread_id_;
        exit(-1);
    } else {
        t_loop_in_this_thread = this;
    }
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assert_in_loop_thread();
    looping_ = true;
    quit_ = false;
    LOG_INFO << "EventLoop " << this << " start loop";

    // pool or epoll
    while (!quit_) {
        active_channels_.clear();
        poller_->poll(kPollTimeMs, &active_channels_);

        for (ChannelList::iterator iter = active_channels_.begin(), end = active_channels_.end(); iter != end; ++iter) {
            (*iter)->handle_event();    // 监听到事件发生，执行用户回调
        }
    }

    LOG_INFO << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;

    // wakeup()
}

void EventLoop::assert_in_loop_thread() {
    if (!is_in_loop_thread()) {
        abort_not_in_loop_thread();
    }
}

bool EventLoop::is_in_loop_thread() const {
    return thread_id_ == util::current_thread::get_tid();
}

void EventLoop::update_channel(Channel* channel) {
    assert(channel->onwer_loop() == this);
    assert_in_loop_thread();
    poller_->update_channel(channel);
}

void EventLoop::abort_not_in_loop_thread() {

}

}
}