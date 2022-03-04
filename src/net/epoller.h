#ifndef EPOLLER_H_
#define EPOLLER_H_

#include <map>
#include <vector>

#include "src/util/noncopyable.h"
#include "src/net/event_loop.h"
#include "src/util/timestamp.h"

struct epoll_event;

namespace xraft {
namespace net {

class Channel;

class EPoller : util::Noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    EPoller(EventLoop* loop);

    ~EPoller();

    // poll IO事件
    util::Timestamp poll(int timeout_ms, ChannelList* activate_channels);

    // 修改感兴趣的IO事件
    void update_channel(Channel* channel);

    void remove_channel(Channel* channel);

    void assert_in_loop_thread() {
        owner_loop_->assert_in_loop_thread();
    }

private:
    using EventList = std::vector<struct epoll_event>;
    using ChannelMap = std::map<int, Channel*>;

    static const int kInitEventListSize = 16;

    void fill_activate_channels(int event_num, ChannelList* activate_channels) const;
    
    void update(int operation, Channel* channel);
    
    EventLoop* owner_loop_;
    int epollfd_;
    EventList events_;       // 待监听的所有channel
    ChannelMap channels_;

};

}
}

#endif