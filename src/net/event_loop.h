#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include <atomic>
#include <sys/types.h>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>

#include "src/util/noncopyable.h"
#include "src/net/timer_id.h"
#include "src/net/callback.cpp"
#include "src/net/timer_queue.h"

namespace xraft {
namespace net {

class Channel;
class Poller;

/**
 * EventLoop 事件循环，创建该对象的线程是IO线程，每个线程仅有一个EventLoop
 * 主要功能是运行loop()函数。
 */
class EventLoop : util::Noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();

    ~EventLoop();

    void loop();

    void quit();

    void assert_in_loop_thread();

    bool is_in_loop_thread() const;

    void update_channel(Channel* channel);

    // 对其他线程提供了一个线程安全的调用方法
    void run_in_loop(Functor cb);

    void queue_in_loop(Functor cb);

    void wakeup();

    TimerId run_at(util::Timestamp time, TimerCallback cb);

    TimerId run_after(double delay, TimerCallback cb);

    TimerId run_every(double interval, TimerCallback cb);

private:
    using ChannelList = std::vector<Channel*>;

    void abort_not_in_loop_thread();

    void handle_read();

    void do_pending_functors();

    const pid_t thread_id_;
    std::atomic_bool looping_;
    std::atomic_bool quit_;
    std::atomic_bool calling_pending_functors_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timer_queue_;
    ChannelList active_channels_;
    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel_;
    std::mutex mutex_;
    std::vector<Functor> pending_functors_;
};

}
}

#endif