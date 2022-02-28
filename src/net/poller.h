#ifndef POLLER_H_
#define POLLER_H_

#include <vector>
#include <map>
#include <poll.h>

#include "src/net/event_loop.h"
#include "src/util/timestamp.h"
#include "src/util/noncopyable.h"

namespace xraft {
namespace net {

class Channel;

/**
 * 封装IO复用
 * 
 */
class Poller : util::Noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);

    ~Poller();

    // poll IO事件
    util::Timestamp poll(int timeout_ms, ChannelList* activate_channels);

    // 修改感兴趣的IO事件
    void update_channel(Channel* channel);

    void assert_in_loop_thread() {
        owner_loop_->assert_in_loop_thread();
    }

private:
    using PollFdList = std::vector<struct pollfd>;
    using ChannelMap = std::map<int, Channel*>;

    void fill_activate_channels(int event_num, ChannelList* activate_channels) const;
    
    EventLoop* owner_loop_;
    PollFdList poll_fds_;       // 待监听的所有channel
    ChannelMap channels_;
};

}
}


#endif