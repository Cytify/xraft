#include "src/net/event_loop.h"

#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <signal.h>

#include "src/util/current_thread.h"
#include "src/log/async_log.h"
#include "src/log/logger.h"
#include "src/net/channel.h"
#include "src/net/poller.h"

namespace xraft {
namespace net {

namespace {

// 标记当前线程的event loop 对象
__thread EventLoop* t_loop_in_this_thread = nullptr;

const int kPollTimeMs = 10000;

int create_event_fd() {
    int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd < 0) {
        LOG_ERROR << "eventfd create failed";
        abort();
    }
    return event_fd;
}

class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        signal(SIGPIPE, SIG_IGN);
    }
};

}

IgnoreSigPipe init_signal;

EventLoop::EventLoop() : 
    thread_id_(util::current_thread::get_tid()), looping_(false),
    quit_(false), calling_pending_functors_(false), poller_(new Poller(this)), 
    timer_queue_(new TimerQueue(this)), wakeup_fd_(create_event_fd()),
    wakeup_channel_(new Channel(this, wakeup_fd_)) {
    if (t_loop_in_this_thread) {
        LOG_ERROR << "EventLoop already exists in this thread, thread id: " << thread_id_;
        exit(-1);
    } else {
        t_loop_in_this_thread = this;
    }

    wakeup_channel_->set_read_callback(std::bind(&EventLoop::handle_read, this));
    wakeup_channel_->enable_read();
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
        poll_return_time_ = poller_->poll(kPollTimeMs, &active_channels_);

        for (ChannelList::iterator iter = active_channels_.begin(), end = active_channels_.end(); iter != end; ++iter) {
            (*iter)->handle_event(poll_return_time_);    // 监听到事件发生，执行用户回调
        }
        do_pending_functors();
    }

    LOG_INFO << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!is_in_loop_thread()) {
        wakeup();
    }
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

void EventLoop::remove_channel(Channel* channel) {
    assert(channel->onwer_loop() == this);
    assert_in_loop_thread();
    poller_->remove_channel(channel);
}

void EventLoop::run_in_loop(Functor cb) {
    if (is_in_loop_thread()) {
        cb();
    } else {
        queue_in_loop(cb);
    }
}

void EventLoop::queue_in_loop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_functors_.push_back(cb);
    }

    if (!is_in_loop_thread() || calling_pending_functors_) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

TimerId EventLoop::run_at(util::Timestamp time, TimerCallback cb) {
    return timer_queue_->add_timer(cb, time, 0.0);
}

TimerId EventLoop::run_after(double delay, TimerCallback cb) {
    util::Timestamp time(util::add_time(util::Timestamp::now(), delay));
    return run_at(time, cb);
}

TimerId EventLoop::run_every(double interval, TimerCallback cb) {
    util::Timestamp time(util::add_time(util::Timestamp::now(), interval));
    return timer_queue_->add_timer(cb, time, interval);
}

void EventLoop::abort_not_in_loop_thread() {
    LOG_ERROR << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << thread_id_
              << ", current thread id = " << util::current_thread::get_tid();
    abort();
}

void EventLoop::handle_read() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::do_pending_functors() {
    std::vector<Functor> functors;
    calling_pending_functors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pending_functors_);
    }
    
    int n = functors.size();
    for (int i = 0; i < n; ++i) {
        functors[i]();
    }
    calling_pending_functors_ = false;
}

}
}