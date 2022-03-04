#include "src/net/timer_queue.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "src/net/event_loop.h"
#include "src/net/timer_id.h"
#include "src/net/timer.h"
#include "src/net/channel.h"
#include "src/log/logger.h"

namespace xraft {
namespace net {

namespace {

int create_timerfd() {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_ERROR << "fail in timerfd create";
    }

    return timerfd;
}

struct timespec how_much_time_from_now(util::Timestamp when) {
    int64_t micro_seconds = when.get_micro_seconds_since_epoch() - util::Timestamp::now().get_micro_seconds_since_epoch();
    if (micro_seconds < 100) {
        micro_seconds = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(micro_seconds / util::Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((micro_seconds % util::Timestamp::kMicroSecondsPerSecond) * 1000);

    return ts;
}

void reset_timerfd(int timerfd, util::Timestamp expiration) {
    struct itimerspec new_value;
    bzero(&new_value, sizeof(new_value));
    new_value.it_value = how_much_time_from_now(expiration);
    int ret = timerfd_settime(timerfd, 0, &new_value, NULL);
    if (ret != 0) {
        LOG_ERROR << "timerfd_settime() error";
    }
}

void read_timerfd(int timerfd, util::Timestamp now) {
    uint64_t how_many;
    ssize_t n = read(timerfd, &how_many, sizeof(how_many));
    LOG_INFO << "TimerQueue::handle_read() " << how_many << " at " << now.to_string();
    if (n != sizeof(how_many)) {
        LOG_ERROR << "TimerQueue::handle_read() reads " << n << " bytes instead of 8";
    }
}

}

TimerQueue::TimerQueue(EventLoop* loop)
    : timerfd_(create_timerfd()), loop_(loop), timerfd_channel_(loop, timerfd_), timers_() {
    timerfd_channel_.set_read_callback(std::bind(&TimerQueue::handle_read, this));
    timerfd_channel_.enable_read();
}

TimerQueue::~TimerQueue() {
    timerfd_channel_.disable_all();
//    timerfd_channel_.remove()
    close(timerfd_);
    for (const Entry& timer : timers_) {
        delete timer.second;
    }
}

TimerId TimerQueue::add_timer(TimerCallback cb, util::Timestamp when, double interval) {
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->run_in_loop(std::bind(&TimerQueue::add_timer_in_loop, this, timer));
    return TimerId(timer, timer->get_sequence());
}

void TimerQueue::cancel(TimerId timer_id) {
    loop_->run_in_loop(std::bind(&TimerQueue::cancel_in_loop, this, timer_id));
}

void TimerQueue::cancel_in_loop(TimerId timer_id) {
    loop_->assert_in_loop_thread();
    assert(timers_.size() == active_timers_.size());
    ActiveTimer timer(timer_id.timer_, timer_id.sequence_);
    ActiveTimerSet::iterator iter = active_timers_.find(timer);
    if (iter != active_timers_.end()) {
        size_t n = timers_.erase(Entry(iter->first->get_expiration(), iter->first));
        assert(n == 1);
        delete iter->first;
        active_timers_.erase(iter);
    } else if (calling_expired_timers_) {
        canceling_timers_.insert(timer);
    }
    assert(timers_.size() == active_timers_.size());
}

void TimerQueue::handle_read() {
    loop_->assert_in_loop_thread();
    util::Timestamp now = util::Timestamp::now();
    read_timerfd(timerfd_, now);

    std::vector<Entry> expired = get_expired(now);
    calling_expired_timers_ = true;
    canceling_timers_.clear();
    for (std::vector<Entry>::iterator iter = expired.begin(), end = expired.end(); iter != end; ++iter) {
        iter->second->run();
    }
    calling_expired_timers_ = false;
    reset(expired, now);
}

void TimerQueue::add_timer_in_loop(Timer* timer) {
    loop_->assert_in_loop_thread();
    bool earliest_changed = insert(timer);

    if (earliest_changed) {
        reset_timerfd(timerfd_, timer->get_expiration());
    }
}

std::vector<TimerQueue::Entry> TimerQueue::get_expired(util::Timestamp now) {
    std::vector<Entry> expired;
    Entry entry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    /**
     * 返回第一个未到期的Timer的迭代器
     * lower_bound返回的是大于等于的，第二个参数设置的是UINTPTR_MAX，最大的指针地址
     * 所以当pair->first相等时，第二个一定小于，所以不会返回pair->first相等的位置
     * 所以now < iter->first是小于，而不是小于等于
     */
    TimerList::iterator iter = timers_.lower_bound(entry);
    assert(iter == timers_.end() || now < iter->first);
    std::copy(timers_.begin(), iter, std::back_inserter(expired));
    timers_.erase(timers_.begin(), iter);

    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, util::Timestamp now) {
    for (std::vector<Entry>::const_iterator iter = expired.begin(), end = expired.end();
         iter != end; ++iter) {
        ActiveTimer timer(iter->second, iter->second->get_sequence());
        if (iter->second->get_repeat() && canceling_timers_.find(timer) == canceling_timers_.end()) {
            iter->second->restart(now);
            insert(iter->second);
        } else {
            delete iter->second;
        }
    }

    // timerfd到期，重新设置timerfd的到期时间
    util::Timestamp next_expire;
    if (!timers_.empty()) {
        next_expire = timers_.begin()->second->get_expiration();
    }
    if (next_expire.valid()) {
        reset_timerfd(timerfd_, next_expire);
    }
}

bool TimerQueue::insert(Timer* timer) {
    bool earliest_changed = false;
    util::Timestamp when = timer->get_expiration();
    TimerList::iterator iter = timers_.begin();
    // 如果是空的队列，或者时间小于最小的定时器，就重新设置timerfd的到期时间
    if (iter == timers_.end() || when < iter->first) {
        earliest_changed = true;
    }

    std::pair<TimerList::iterator, bool> result = timers_.insert(std::make_pair(when, timer));
    assert(result.second);

    return earliest_changed;
}

}
}