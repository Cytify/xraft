#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include <atomic>
#include <sys/types.h>
#include <memory>
#include <vector>

#include "src/util/noncopyable.h"

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
    EventLoop();

    ~EventLoop();

    void loop();

    void quit();

    void assert_in_loop_thread();

    bool is_in_loop_thread() const;

    void update_channel(Channel* channel);

private:
    using ChannelList = std::vector<Channel*>;

    void abort_not_in_loop_thread();

    const pid_t thread_id_;
    std::atomic_bool looping_;
    std::atomic_bool quit_;
    std::unique_ptr<Poller> poller_;
    ChannelList active_channels_;
};

}
}

#endif