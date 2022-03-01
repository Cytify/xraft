#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include <set>
#include <vector>

#include "src/util/noncopyable.h"
#include "src/util/timestamp.h"
#include "src/net/callback.h"
#include "src/net/channel.h"

namespace xraft {
namespace net {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : util::Noncopyable {
public:
    explicit TimerQueue(EventLoop* loop);

    ~TimerQueue();

    TimerId add_timer(TimerCallback cb, util::Timestamp when, double interval);

    void cancel(TimerId timer_id);

private:
    /**
     * 使用set存储所有定时器
     * 为了防止相同时间到期的定时器，所以将其定义为std::pair
     * 或者也可以使用multiset
     */
    using Entry = std::pair<util::Timestamp, Timer*>;
    using TimerList = std::set<Entry>;

    // 当timerfd到期时，被调用
    void handle_read();

    void add_timer_in_loop(Timer* timer);

    // 获取到期的timer
    std::vector<Entry> get_expired(util::Timestamp now);

    void reset(const std::vector<Entry>& expired, util::Timestamp now);

    bool insert(Timer* timer);

    const int timerfd_;
    EventLoop* loop_;
    Channel timerfd_channel_;
    TimerList timers_;
};


}
}


#endif